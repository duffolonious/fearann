--
-- PostgreSQL database dump
--

CREATE TABLE usr_accts (
    uid serial NOT NULL,
    username character varying(32) NOT NULL,
    "password" character varying(40) NOT NULL,
    realname character varying(64),
    email character varying(64),
    creation_date timestamp without time zone DEFAULT ('now'::text)::timestamp(6) with time zone,
    last_login timestamp without time zone,
    last_login_ip character varying(15),
    time_zone character varying(8),
    roles integer DEFAULT 0 NOT NULL,
    status integer DEFAULT 0 NOT NULL,
    banned_until timestamp without time zone,
    comments character varying(1024),
    number_logins integer DEFAULT 0 NOT NULL
);


CREATE TABLE usr_chars (
    creation_date timestamp without time zone DEFAULT ('now'::text)::timestamp(6) with time zone,
    last_login timestamp without time zone,
    status integer DEFAULT 0,
    comments character varying(1024),
    area character varying(64) NOT NULL,
    pos1 double precision NOT NULL,
    pos2 double precision NOT NULL,
    pos3 double precision NOT NULL,
    rot double precision NOT NULL,
    race character varying(32) NOT NULL,
    gender character varying(1) NOT NULL,
    class character varying(32) NOT NULL,
    uid integer,
    cid bigint DEFAULT nextval('public.entities_id_seq'::text) NOT NULL,
    charname character varying(64) NOT NULL,
    number_logins integer DEFAULT 0,
    time_playing interval DEFAULT '00:00:00'::interval
);


CREATE TABLE contact_list (
    charname character varying(64) NOT NULL,
    contact_charname character varying(64) NOT NULL,
    creation_date timestamp without time zone DEFAULT ('now'::text)::timestamp(6) with time zone NOT NULL,
    "type" integer NOT NULL,
    comments character varying(1024),
    cid bigint,
    contact_cid bigint
);


CREATE TABLE player_stats (
    health integer,
    stamina integer,
    magic integer,
    ab_con integer,
    ab_str integer,
    ab_dex integer,
    ab_int integer,
    ab_wis integer,
    ab_cha integer,
    charname character varying(64) NOT NULL,
    gold integer,
    xp integer,
    level integer
);


CREATE SEQUENCE entities_id_seq
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


CREATE TABLE world (
    "time" integer
);


CREATE TABLE entities (
    creation_date timestamp without time zone NOT NULL,
    id bigint DEFAULT nextval('public.entities_id_seq'::text) NOT NULL,
    area character varying(64) NOT NULL,
    pos1 double precision NOT NULL,
    pos2 double precision NOT NULL,
    pos3 double precision NOT NULL,
    rot double precision NOT NULL,
    "type" character varying(32) NOT NULL,
    subtype character varying(32) NOT NULL,
    "owner" character varying(64)
);


CREATE TABLE creatures (
    creation_date timestamp without time zone NOT NULL,
    id bigint DEFAULT nextval('public.entities_id_seq'::text) NOT NULL,
    area character varying(64) NOT NULL,
    pos1 double precision NOT NULL,
    pos2 double precision NOT NULL,
    pos3 double precision NOT NULL,
    rot double precision NOT NULL,
    "type" character varying(32) NOT NULL,
    subtype character varying(32) NOT NULL,
    "owner" character varying(64)
);


ALTER TABLE ONLY usr_accts
    ADD CONSTRAINT user_accounts_pkey PRIMARY KEY (uid);

ALTER TABLE ONLY usr_accts
    ADD CONSTRAINT user_accounts_username_key UNIQUE (username);

ALTER TABLE ONLY usr_chars
    ADD CONSTRAINT usr_chars_pkey PRIMARY KEY (cid);

ALTER TABLE ONLY usr_chars
    ADD CONSTRAINT usr_chars_cid_key UNIQUE (cid);

ALTER TABLE ONLY usr_chars
    ADD CONSTRAINT usr_chars_charname_key UNIQUE (charname);

ALTER TABLE ONLY usr_chars
    ADD CONSTRAINT uid_chk FOREIGN KEY (uid) REFERENCES usr_accts(uid) MATCH FULL;

ALTER TABLE ONLY contact_list
    ADD CONSTRAINT charname_chk FOREIGN KEY (charname) REFERENCES usr_chars(charname) MATCH FULL;

ALTER TABLE ONLY contact_list
    ADD CONSTRAINT contact_charname_chk FOREIGN KEY (contact_charname) REFERENCES usr_chars(charname) MATCH FULL;

ALTER TABLE ONLY player_stats
    ADD CONSTRAINT charname_chk FOREIGN KEY (charname) REFERENCES usr_chars(charname) MATCH FULL;

ALTER TABLE ONLY contact_list
    ADD CONSTRAINT contact_cid_chk FOREIGN KEY (contact_cid) REFERENCES usr_chars(cid) MATCH FULL;

ALTER TABLE ONLY contact_list
    ADD CONSTRAINT cid_chk FOREIGN KEY (cid) REFERENCES usr_chars(cid) MATCH FULL;


SELECT pg_catalog.setval('usr_accts_uid_seq', 1, true);

SELECT pg_catalog.setval('entities_id_seq', 1, true);


INSERT INTO world(time) VALUES(0);

--------------------------------
-- Functions to make life easier

-- Uncomment if language is not yet installed.
-- CREATE LANGUAGE "plpgsql";

-- create a new account
CREATE FUNCTION createUser(
		n_username character varying(32),
		n_password character varying(40),
		n_realname character varying(64),
		n_email character varying(64),
		n_comments character varying(1024)
	) RETURNS boolean
	AS $_$
-- todo: put stuff in usr_accts
BEGIN
	INSERT INTO usr_accts (username,"password",realname,email,comments) VALUES (n_username,n_password,n_realname,n_email,n_comments);
	RETURN TRUE;
END;
$_$
LANGUAGE plpgsql;

-- like you outlined above
CREATE FUNCTION createChar(character varying) RETURNS boolean
    AS $_$
BEGIN
END;
$_$
LANGUAGE plpgsql;

-- create an object - like a bottle, or ring
CREATE FUNCTION createEntity(character varying) RETURNS boolean
    AS $_$
BEGIN
END;
$_$
LANGUAGE plpgsql;

-- create a rabbit for us to kill (or talk to, if you are lonely)
CREATE FUNCTION createCreature(character varying) RETURNS boolean
    AS $_$
BEGIN
END;
$_$
LANGUAGE plpgsql;

