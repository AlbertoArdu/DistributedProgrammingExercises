use Esame_PD1;
drop table utenti;
drop table prenotazioni;
drop table attivita;

create table utenti(
	nome_utente VARCHAR(40) NOT NULL,
	password VARCHAR(30) NOT NULL,
	PRIMARY KEY (nome_utente)
);

create table prenotazioni(
	nome_utente VARCHAR(40) NOT NULL,
	cod_attivita VARCHAR(10) NOT NULL,
	num_partecipanti INT NOT NULL,
	PRIMARY KEY (nome_utente,cod_attivita)
);

create table attivita(
	codice VARCHAR(10) NOT NULL,
	nome VARCHAR(30) NOT NULL,
	posti_disponibili INT NOT NULL,
	posti_totali INT NOT NULL,
	PRIMARY KEY(codice)
);

insert into utenti(nome_utente,password)
	values('u1','p1');

insert into utenti(nome_utente,password)
	values('u2','p2');

insert into utenti(nome_utente,password)
	values('u3','p3');

insert into attivita(codice,nome,posti_disponibili,posti_totali)
	values('Att1','Calcetto a 5',1,6);

insert into attivita(codice,nome,posti_disponibili,posti_totali)
	values('Att2','Tennis Doppio',6,8);

insert into attivita(codice,nome,posti_disponibili,posti_totali)
	values('Att3','Danze Caraibiche',2,4);

insert into prenotazioni(nome_utente,cod_attivita,num_partecipanti)
	values('u1','Att1',2);

insert into prenotazioni(nome_utente,cod_attivita,num_partecipanti)
	values('u1','Att2',2);

insert into prenotazioni(nome_utente,cod_attivita,num_partecipanti)
	values('u2','Att3',1);

insert into prenotazioni(nome_utente,cod_attivita,num_partecipanti)
	values('u2','Att1',3);

insert into prenotazioni(nome_utente,cod_attivita,num_partecipanti)
	values('u3','Att3',1);

insert into prenotazioni(nome_utente,cod_attivita,num_partecipanti)
	values('u3','Att2',4);