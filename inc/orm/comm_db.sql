
drop table if EXISTS Users;

create table Users(
  id SERIAL,
  username varchar(128),
  nickname varchar(256),
  avatar varchar(256),
  passwd varchar(256),
  salt varchar(256)
);

