
drop table if EXISTS Users;

create table Users(
  id SERIAL,
  username varchar(128),
  nickname varchar(256),
  avatar varchar(256),
  passwd varchar(256),
  salt varchar(256)
);

create table Transitions(
  order_no SERIAL,
  timestampx varchar(32),
  username varchar(64),
  price int,
  status varchar(12)
);
