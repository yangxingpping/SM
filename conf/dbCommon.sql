
DROP table if EXISTS users;
DROP table if EXISTS token;
DROP table if EXISTS sysadmin;

create table users(name varchar(255), passwd varchar(255),  phone varchar(64), expiredate datetime);
create table token(name varchar(255), token varchar(255));
create table sysadmin(user varchar(255), phone varchar(255), admin varchar(255), role int);