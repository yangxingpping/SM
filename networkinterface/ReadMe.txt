网络库说明:
    网络库整合使用nanomsg, http, websocket, 基于各个协议的特点 分成一下两类
    1, http(post,get) 使用 HttpGetPost 来实现(消息使用url来区分)
    2, websocket, nanomsg 消息头包含在消息的开始固定长度字节来区分，使用base64编码