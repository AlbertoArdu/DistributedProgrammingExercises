<?php

/**
* 
Classe per gestire il db:
	- il costruttore apre la connessione
	- il distruttore chiude la connessione
	- funzioni utili all'applicazione
*/
class ClassDB
{
	private $host = "localhost";
	private $nome = "Esame_PD1";
	private $user = array('nome' => 'guest', 'password' => 'password');
	private $conn;
	private $in_transazione = false;
	public $status;

	function __construct()
	{
		if($this->conn = mysqli_connect($this->host, $this->user['nome'], $this->user['password'], $this->nome))
			$this->status = 1;
		else
			$this->status = 0;	
	}
	function __destruct(){
		mysqli_close($this->conn);
	}

	public function BeginTransaction(){
		mysqli_autocommit($this->conn,false);
		$this->in_transazione = true;
	}

	public function Commit(){
		if($this->in_transazione)
			return mysqli_commit($this->conn);
		else
			return 0;
	}

	public function Rollback(){
		if($this->in_transazione)
			return mysqli_rollback($this->conn);
		else
			return 0;
	}
	
	public function EndTransaction(){
		if(!$this->in_transazione)
			return 0;
		if ($this->Commit()){
			mysqli_autocommit($this->conn, true);
			$this->in_transazione = false;
			return 1;
		}else{
			$this->Rollback();
			autocommit($this->conn, true);
			$this->in_transazione = false;
			return 0;
		}
	}
	
	public function userExists($user){
		$user = mysqli_real_escape_string($this->conn,$user);
		$query = "SELECT * FROM utenti WHERE nome_utente = '".$user."'";

		if($this->in_transazione){
			$query .= " FOR UPDATE;";
		}else{
			$query .= ";";
		}
		$res = mysqli_query($this->conn,$query, MYSQLI_USE_RESULT);

		if($res){
			$tupla = mysqli_fetch_array($res,MYSQLI_ASSOC);
			mysqli_free_result($res);
			return $tupla;
		}else{
			return 0;
		}
	}
	public function aggiungiUtente($user,$pass){
		$user = mysqli_real_escape_string($this->conn,$user);
		$pass = mysqli_real_escape_string($this->conn,$pass);
		$query = "INSERT into utenti(nome_utente,password) VALUES ('".$user."','".$pass."');";
		return mysqli_query($this->conn,$query);
	}
	public function CercaAttivita($codice = ''){
		$query = "SELECT * FROM attivita";

		if($codice != ''){
			$codice = mysqli_real_escape_string($this->conn,$codice);
			$query .= " WHERE codice = '".$codice."'";
		}

		if($this->in_transazione){
			$query .= " FOR UPDATE;";
		}else{
			$query .= " ORDER BY posti_disponibili DESC;";
		}

		$res = mysqli_query($this->conn,$query);
		if(!$res){
			$GLOBALS['error_message'] = "Errore nel cercare le attivita".mysqli_error($this->conn).$query;
			return 0;
		}

		//Se cerco sulla chiave primaria ci sarà solamente un risultato
		if($codice != ''){
			$tuple = mysqli_fetch_array($res,MYSQLI_ASSOC);
		}elseif(($n = mysqli_num_rows($res)) == 0){
			$tuple = 0;
		}else{
			for ( $i = 0; $i < $n; $i++) {
				$tuple[$i] = mysqli_fetch_array($res,MYSQLI_ASSOC);
			}
		}
		mysqli_free_result($res);

		return $tuple;
	}

	public function Query($query){
		return mysqli_query($this->db,$query);
	}
	public function updateDisponibili($codice,$n){
		$codice = mysqli_real_escape_string($this->conn,$codice);
		$query = "UPDATE attivita SET posti_disponibili = ".$n." WHERE codice = '".$codice."';";
		$res = mysqli_query($this->conn,$query);
		$GLOBALS['error_message'] = mysqli_error($this->conn);
		
		return $res;
	}

	public function inserisciPrenotazione($user,$codice,$n){
		$user = mysqli_real_escape_string($this->conn,$user);
		$codice = mysqli_real_escape_string($this->conn,$codice);
		$n = mysqli_real_escape_string($this->conn,$n);
		$this->BeginTransaction();
		if ($this->prenotazioneEsiste($user,$codice)!=0) {
			$this->Rollback();
			$GLOBALS['error_message'] = "Prenotazione già fatta per questa attività";
			return 0;
		}
		$query = 'INSERT INTO prenotazioni (nome_utente,cod_attivita,num_partecipanti) VALUES ("'.$user.'", "'.$codice.'", "'.$n.'");';
		$res = mysqli_query($this->conn,$query);
		$GLOBALS['error_message'] = mysqli_error($this->conn);
		$this->EndTransaction();
		return $res;	
	}

	public function cercaPrenotazioni($user){
		$query = "SELECT codice, nome, num_partecipanti FROM attivita, prenotazioni ";
		$query .= "WHERE attivita.codice = prenotazioni.cod_attivita AND nome_utente = '".$user."';";

		$res = mysqli_query($this->conn,$query);
		if(($n = mysqli_num_rows($res)) == 0){
			$tuple = 0;
		}else{
			for ( $i = 0; $i < $n; $i++) {
				$tuple[$i] = mysqli_fetch_array($res,MYSQLI_ASSOC);
			}
		}
		mysqli_free_result($res);
		return $tuple;
	}

	public function prenotazioneEsiste($user,$codice){
		$query = "SELECT * FROM prenotazioni WHERE nome_utente = '".$user;
		$query .= "' and cod_attivita = '".$codice."' FOR UPDATE;";
		$res = mysqli_query($this->conn,$query);
		if(($n = mysqli_num_rows($res)) == 0){
			$tuple = 0;
		}else{
			$tuple = mysqli_fetch_array($res,MYSQLI_ASSOC);
		}
		mysqli_free_result($res);
		return $tuple;
	}
	public function eliminaPrenotazione($user,$codice){
		$pren = $this -> prenotazioneEsiste($user,$codice);
		if(!$pren)
			return 0;
		$query = "DELETE FROM prenotazioni WHERE nome_utente = '".$user;
		$query .= "' and cod_attivita = '".$codice."';";
		$res = mysqli_query($this->conn,$query);
		if(!$res){
			return 0;
		}
		$query = "UPDATE attivita SET posti_disponibili = posti_disponibili + ".$pren['num_partecipanti'];
		$query .= " WHERE codice = '".$codice."';";
		$res = mysqli_query($this->conn,$query);
		if(!$res){
			return 0;
		}
		return 1;
	}
}
?>