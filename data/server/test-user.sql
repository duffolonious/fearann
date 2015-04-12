select createUser('testuser', '0e0e68cc27a6334256e0752d1243c4d894e56869', 'John Doe', 'jdoe@fearann.muin', 'test');
insert into usr_chars (area,charname,pos1,pos2,pos3,rot,race,gender,class) values ('tmprotmar', 'testchar', 0.0, 0.0, 0.0, 0.0, 'human', 'm', 'fighter');
insert into player_stats (charname) values ('testchar');
-- merge this update into the insert above
update player_stats set health=10, stamina=10, magic=10, ab_con=3, ab_str=3, ab_dex=3, ab_int=3, ab_wis=3, ab_cha=3, gold=0, xp=0, level=1;
-- where uid is the usr_acct's id
update usr_chars set uid=2;
-- role 1 is a player (2 for admin)
update usr_accts set roles=1;
