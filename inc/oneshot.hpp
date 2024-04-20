// Copyright (c) 2022 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <atomic>
#include <cstdint>
#include <utility>

#ifdef ONESHOT_ASIO_STANDALONE
#include <asio/append.hpp>
#include <asio/associated_cancellation_slot.hpp>
#include <asio/post.hpp>
namespace oneshot
{
namespace net    = asio;
using error_code = std::error_code;
} // namespace oneshot
#else
#include <boost/asio/append.hpp>
#include <boost/asio/associated_cancellation_slot.hpp>
#include <boost/asio/post.hpp>
namespace oneshot
{
namespace net    = boost::asio;
using error_code = boost::system::error_code;
} // namespace oneshot
#endif

namespace oneshot
{
enum class errc
{
    no_state = 1,
    cancelled,
    unready,
    broken_sender,
    duplicate_wait_on_receiver,
};
} // namespace oneshot

namespace std
{
template<>
struct is_error_code_enum<oneshot::errc> : true_type
{
};
} // namespace std

namespace oneshot
{
inline const std::error_category& oneshot_category()
{
    static const struct : std::error_category
    {
        const char* name() const noexcept override
        {
            return "oneshot";
        }

        std::string message(int ev) const override
        {
            switch (static_cast<errc>(ev))
            {
                case errc::no_state:
                    return "No associated state";
                case errc::cancelled:
                    return "Cancelled";
                case errc::unready:
                    return "Unready";
                case errc::broken_sender:
                    return "Broken sender";
                case errc::duplicate_wait_on_receiver:
                    return "Duplicate wait on receiver";
                default:
                    return "Unknown error";
            }
        }
    } category;

    return category;
};

inline std::error_code make_error_code(errc e)
{
    return { static_cast<int>(e), oneshot_category() };
}

class error : public std::system_error
{
  public:
    using std::system_error::system_error;
};

namespace detail
{
struct wait_op
{
    virtual void shutdown() noexcept  = 0;
    virtual void complete(error_code) = 0;
    virtual ~wait_op()                = default;
};

template<class Executor, class Handler>
class wait_op_model final : public wait_op
{
    net::executor_work_guard<Executor> work_guard_;
    Handler handler_;

  public:
    wait_op_model(Executor e, Handler handler)
        : work_guard_(std::move(e))
        , handler_(std::move(handler))
    {
    }

    [[nodiscard]] auto get_cancellation_slot() const noexcept
    {
        return net::get_associated_cancellation_slot(handler_);
    }

    static wait_op_model* construct(Executor e, Handler handler)
    {
        auto halloc = net::get_associated_allocator(handler);
        auto alloc  = typename std::allocator_traits<
            decltype(halloc)>::template rebind_alloc<wait_op_model>(halloc);
        using traits = std::allocator_traits<decltype(alloc)>;
        auto pmem    = traits::allocate(alloc, 1);

        try
        {
            return new (pmem) wait_op_model(std::move(e), std::move(handler));
        }
        catch (...)
        {
            traits::deallocate(alloc, pmem, 1);
            throw;
        }
    }

    static void destroy(
        wait_op_model* self,
        net::associated_allocator_t<Handler> halloc)
    {
        auto alloc = typename std::allocator_traits<
            decltype(halloc)>::template rebind_alloc<wait_op_model>(halloc);
        self->~wait_op_model();
        auto traits = std::allocator_traits<decltype(alloc)>();
        traits.deallocate(alloc, self, 1);
    }

    void complete(error_code ec) override
    {
        get_cancellation_slot().clear();
        auto g = std::move(work_guard_);
        auto h = std::move(handler_);
        destroy(this, net::get_associated_allocator(h));
        net::post(g.get_executor(), net::append(std::move(h), ec));
    }

    void shutdown() noexcept override
    {
        get_cancellation_slot().clear();
        destroy(this, net::get_associated_allocator(this->handler_));
    }
};

template<typename T>
struct storage;

template<>
struct storage<void>
{
    void construct() noexcept
    {
    }

    void destroy() noexcept
    {
    }

    void* object() noexcept
    {
        return nullptr;
    }
};

template<typename T>
struct storage
{
    alignas(T) char payload_[sizeof(T)];

    template<typename... Args>
    void construct(Args&&... args)
    {
        new (&payload_) T(std::forward<Args>(args)...);
    }

    void destroy() noexcept
    {
        std::destroy_at(object());
    }

    T* object() noexcept
    {
        return std::launder(reinterpret_cast<T*>(&payload_));
    }
};

template<typename T>
class shared_state
{
    enum : uint8_t
    {
        empty    = 0,
        engaged  = 1,
        waiting  = 2,
        sent     = 3,
        detached = 4
    };

    std::atomic<uint8_t> state_{ empty };
    [[no_unique_address]] storage<T> storage_;
    void (*deleter_)(shared_state*){ nullptr };
    wait_op* wait_op_{ nullptr };

  public:
    shared_state(void (*deleter)(shared_state*)) noexcept
        : deleter_{ deleter }
    {
    }

    shared_state(const shared_state&) = delete;
    shared_state(shared_state&&)      = delete;

    template<typename... Args>
    void send(Args&&... args)
    {
        storage_.construct(std::forward<Args>(args)...);

        // possible vals: empty, waiting, detached(receiver)
        auto prev = state_.fetch_add(1, std::memory_order_release);

        if (prev == detached)
        {
            storage_.destroy();
            return deleter_(this);
        }

        if (prev == waiting)
        {
            std::atomic_thread_fence(std::memory_order_acquire);
            wait_op_->complete({});
        }
    }

    void sender_detached() noexcept
    {
        // possible vals: empty, waiting, detached(receiver)
        auto prev = state_.exchange(detached, std::memory_order_relaxed);

        if (prev == detached)
            return deleter_(this);

        if (prev == waiting)
        {
            std::atomic_thread_fence(std::memory_order_acquire);
            wait_op_->complete(errc::broken_sender);
        }
    }

    template<typename CompletionToken>
    auto async_wait(CompletionToken&& token)
    {
        return net::async_initiate<decltype(token), void(error_code)>(
            [this](auto handler)
            {
                auto exec = get_associated_executor(handler);

                using handler_type = std::decay_t<decltype(handler)>;
                using model_type  = wait_op_model<decltype(exec), handler_type>;
                model_type* model = model_type ::construct(
                    std::move(exec), std::forward<decltype(handler)>(handler));
                auto c_slot = model->get_cancellation_slot();
                if (c_slot.is_connected())
                {
                    c_slot.assign(
                        [this](net::cancellation_type type)
                        {
                            if (type != net::cancellation_type::none)
                            {
                                // possible vals: waiting, sent,
                                // detached(sender)
                                auto prev = state_.exchange(
                                    empty, std::memory_order_relaxed);

                                if (prev == waiting)
                                {
                                    this->wait_op_->complete(errc::cancelled);
                                    this->wait_op_ = nullptr;
                                }
                                else // prev has been sent or detached(sender)
                                {
                                    state_.store(
                                        prev, std::memory_order_relaxed);
                                }
                            }
                        });
                }

                if (this->wait_op_)
                    return model->complete(errc::duplicate_wait_on_receiver);

                wait_op_ = model;

                // possible vals: empty, engaged, detached(sender)
                auto prev = state_.exchange(waiting, std::memory_order_release);

                if (prev == detached)
                {
                    state_.store(prev, std::memory_order_relaxed);
                    model->complete(errc::broken_sender);
                }
                else if (prev == engaged)
                {
                    state_.store(prev, std::memory_order_relaxed);
                    model->complete({});
                }
            },
            token);
    }

    void receiver_detached() noexcept
    {
        // possible vals: empty, engaged, sent, detached(sender)
        auto prev = state_.exchange(detached, std::memory_order_relaxed);

        if (prev == detached)
            return deleter_(this);

        if (prev == engaged || prev == sent)
        {
            std::atomic_thread_fence(std::memory_order_acquire);
            storage_.destroy();
            return deleter_(this);
        }
    }

    bool is_ready() const noexcept
    {
        auto state = state_.load(std::memory_order_relaxed);
        return state == sent || state == engaged;
    }

    T* get_stored_object() noexcept
    {
        auto state = state_.load(std::memory_order_acquire);

        if (state == sent || state == engaged)
            return storage_.object();

        return nullptr;
    }
};
} // namespace detail
template<typename T>
class sender
{
    detail::shared_state<T>* shared_state_{ nullptr };

  public:
    sender() noexcept = default;

    sender(detail::shared_state<T>* shared_state) noexcept
        : shared_state_{ shared_state }
    {
    }

    sender(sender&& other) noexcept
    {
        std::swap(shared_state_, other.shared_state_);
    }

    sender& operator=(sender&& other) noexcept
    {
        std::swap(shared_state_, other.shared_state_);
        return *this;
    }

    template<typename... Args>
    void send(Args&&... args)
    {
        if (!shared_state_)
            throw error{ errc::no_state };

        shared_state_->send(std::forward<Args>(args)...);

        shared_state_ = nullptr;
    }

    ~sender()
    {
        if (shared_state_)
            shared_state_->sender_detached();
    }
};

template<typename T>
class receiver
{
    detail::shared_state<T>* shared_state_{ nullptr };

  public:
    receiver() noexcept = default;

    receiver(detail::shared_state<T>* shared_state) noexcept
        : shared_state_{ shared_state }
    {
    }

    receiver(receiver&& other) noexcept
    {
        std::swap(shared_state_, other.shared_state_);
    }

    receiver& operator=(receiver&& other) noexcept
    {
        std::swap(shared_state_, other.shared_state_);
        return *this;
    }

    template<typename CompletionToken>
    auto async_wait(CompletionToken&& token)
    {
        if (!shared_state_)
            throw error{ errc::no_state };

        return shared_state_->async_wait(std::forward<CompletionToken>(token));
    }

    bool is_ready() const
    {
        if (!shared_state_)
            throw error{ errc::no_state };

        return shared_state_->is_ready();
    }

    decltype(auto) get() const
    {
        static_assert(!std::is_same_v<T, void>, "Only for non void receivers");

        if (!shared_state_)
            throw error{ errc::no_state };

        if (auto* p = shared_state_->get_stored_object())
            return std::add_lvalue_reference_t<T>(*p);

        throw error{ errc::unready };
    }

    ~receiver()
    {
        if (shared_state_)
            shared_state_->receiver_detached();
    }
};

template<typename T, typename Allocator = std::allocator<T>>
inline std::pair<sender<T>, receiver<T>> create(Allocator alloc = {})
{
    struct wrapper
    {
        detail::shared_state<T> shared_state_;
        [[no_unique_address]] Allocator alloc_;

        wrapper(void (*deleter)(detail::shared_state<T>*), Allocator alloc)
            : shared_state_{ deleter }
            , alloc_{ alloc }
        {
        }
    };

    using r_alloc_t = typename std::allocator_traits<
        Allocator>::template rebind_alloc<wrapper>;
    using traits_t = std::allocator_traits<r_alloc_t>;
    auto r_alloc   = r_alloc_t{ alloc };
    auto* p        = traits_t::allocate(r_alloc, 1);
    auto* deleter  = +[](detail::shared_state<T>* shared_state)
    {
        auto* p      = reinterpret_cast<wrapper*>(shared_state);
        auto r_alloc = r_alloc_t{ p->alloc_ }; // copy before destroy
        traits_t::destroy(r_alloc, p);
        traits_t::deallocate(r_alloc, p, 1);
    };
    traits_t::construct(r_alloc, p, deleter, alloc); // noexcept
    return { &p->shared_state_, &p->shared_state_ };
}
} // namespace oneshot
