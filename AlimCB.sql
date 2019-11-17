create or replace directory pathMovies as 'C:\SGBD\';
drop table movies_ext;

create table movies_ext (
    id number(6),
    title varchar(60),
    original_title varchar(60),
    release_date date,
    status varchar(15),
    vote_average number(3,1),
    vote_count number(5),
    runtime number(3),
    certification varchar(5),
    poster_path varchar(100),
    budget number(10),
    tagline varchar(100),
    genres varchar(1000),
    directors varchar(1000),
    actors varchar(1000)
) organization external (
    type oracle_loader
    default directory pathMovies
    access parameters (
        records delimited by '\n'
        characterset "AL32UTF8"
        string sizes are in characters
        fields terminated by 'LA PUTAIN DE FLECHE'
        missing field values are null (
            id unsigned integer external(8),
            title char(60),
            original_title char(60),
            release_date char(10) date_format date mask "YYYY-MM-DD",
            status char(15),
            vote_average unsigned integer external(4),
            vote_count unsigned integer external(8),
            runtime unsigned integer external(4),
            certification char(5),
            poster_path char(100),
            budget unsigned integer external(16),
            tagline char(100),
            genres char(1000),
            directors char(1000),
            actors char(1000)
        )
    )
    location('movies.txt')
)
reject limit unlimited;

select * from movies_ext where id = 534989 or id = 2;