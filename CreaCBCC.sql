drop table artist cascade constraints;
drop table certification cascade constraints;
drop table status cascade constraints;
drop table genre cascade constraints;
drop table movie cascade constraints;
drop table movie_director cascade constraints;
drop table movie_genre cascade constraints;
drop table movie_actor cascade constraints;
drop table users cascade constraints;
drop table critics cascade constraints;

create table artist (
  id   number(7),
  name varchar(23) constraint artist_name$nn not null,
  constraint artist$pk primary key (id)
);

create table certification (
  id          number(1),
  code        varchar(5) constraint cert_code$nn not null
                            constraint cert_code$ck check(code in('G', 'PG', 'PG-13', 'R', 'NC-17')),
  name        varchar(28) constraint cert_name$nn not null,
  definition  varchar(56) constraint cert_definition$nn not null,
  description varchar(122), -- default desc
  constraint cert$pk primary key (id)
);

create table status (
  id          number(1),
  name        varchar(15) constraint status_name$nn not null,
  constraint status$pk primary key (id)
);

create table genre (
  id   number(5),
  name varchar(15) constraint genre_name$nn not null,
  constraint genre$pk primary key (id)
);

create table movie (
  id             number(6),
  title          varchar(60) constraint movie_title$nn not null,
  original_title varchar(60) constraint movie_ot$nn not null,
  status         number(1) constraint movie_status$fk references status(id)
                            constraint movie_status$nn not null,
  release_date   date constraint movie_rd$nn not null,
  vote_average   number(3,1) default 0
                            constraint movie_va$nn not null,
  vote_count     number(5) default 0
                            constraint movie_vc$nn not null,
  certification  number(1) constraint movie_certif$fk references certification(id)
                            constraint movie_certif$nn not null,
  runtime        number(3) constraint movie_runtime$nn not null
                            constraint movie_runtime$ck check(runtime < 420), -- minutes
  poster         blob,
  constraint movie$pk primary key (id)
);

create table movie_director (
  movie    number(6),
  director number(7),
  constraint m_d$pk primary key (movie, director)
);

create table movie_genre (
  movie number(6),
  genre number(5),
  constraint m_g$pk primary key (genre, movie)
  ) ;

create table movie_actor (
  movie number(6),
  actor number(7),
  constraint m_a$pk primary key (movie, actor)
);

create table users (
    id number(9) constraint user$pk primary key,
    name varchar(23) constraint user_name$nn not null
);

create table critics (
    movie number(6),
    users number(9),
    comments varchar(600),
    dateCritic date constraint critic_date$nn not null, -- check date sup ou égale a date film avec un trigger
    constraint critic$pk primary key (movie, users)
);