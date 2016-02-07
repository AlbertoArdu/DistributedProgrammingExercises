<?php

$timeout_conn = 120;

$db = new ClassDB();
if(!$db->status)
	$GLOBALS['error_message'] = "Errore nell'apertura del database";

function test_input($data) {
	$data = trim($data);
	$data = stripslashes($data);
	$data = htmlspecialchars($data);
	return $data;
}

function Redirect(){
	header("HTTP/1.1 307 temporary redirect");
	header("Location: https://".$_SERVER['HTTP_HOST'].$_SERVER['REQUEST_URI']);
	exit();
}

function HttpsAssert(){
	if(!isset($_SERVER['HTTPS']) || $_SERVER['HTTPS']=='off'){
		Redirect();
	}
}

function CheckConnTimeout(){
	$t=time();
	$diff=0;

	if (isset($_SESSION['s220318_time'])){
		$t0=$_SESSION['s220318_time'];
		$diff=($t-$t0);
	}else{
		return 0;
	}

	if (($diff > $GLOBALS['timeout_conn'])) {
		$_SESSION=array();
		// If it's desired to kill the session, also delete the session cookie.
		// Note: This will destroy the session, and not just the session data!
		// PHP using cookies to handle session
		$params = session_get_cookie_params();
		setcookie(session_name(), '', time() - 3600*24, $params["path"], $params["domain"], $params["secure"], $params["httponly"]);
		session_destroy();
		return 0;
	} else {
		$_SESSION['s220318_time'] = time();
		return 1;
	}
}

function Login(){
	$_SESSION = array();

	if (!isset($_POST['user']) || !isset($_POST['pass']))  {
		$GLOBALS['error_message'] = "Nome utente o password mancanti";
		return 0;
	}
	$user = test_input($_POST['user']);
	$pass = test_input($_POST['pass']);

	if($user == "" || $pass == ""){
		$GLOBALS['error_message'] = "Nome utente o password mancanti";
		return 0;
	}

	if($GLOBALS['db']->status){
		$tupla = $GLOBALS['db']->userExists($user);
		if(!$tupla){
			$GLOBALS['error_message'] = "Nome utente inesistente";
			return 0;
		} elseif($tupla['password'] == $pass){
			$_SESSION['s220318_time']=time();
			$_SESSION['s220318_user']=$user;
			return 1;
		}else{
			$GLOBALS['error_message'] = "Password errata";
			return 0;
		}
	}else{
		$GLOBALS['error_message'] = "Errore nell'apertura del db";
		return 0;
	}
}

function Logout(){
	$_SESSION=array();
	// If it's desired to kill the session, also delete the session cookie.
	// Note: This will destroy the session, and not just the session data!
	// PHP using cookies to handle session
	$params = session_get_cookie_params();
	setcookie(session_name(), '', time() - 3600*24, $params["path"], $params["domain"], $params["secure"], $params["httponly"]);
	session_destroy(); // destroy session
	$GLOBALS['loggato']=0;
}

function registraUtente(){

	if (!isset($_POST['user']) || !isset($_POST['pass'])) {
		$GLOBALS['error_message'] = "Nome utente o password mancanti";
		return 0;
	}
	$nuovoNome = test_input($_POST['user']);
	$pass = test_input($_POST['pass']);

	if($nuovoNome == "" || $pass == ""){
		$GLOBALS['error_message'] = "Nome utente o password mancanti";
		return 0;
	}

	if($GLOBALS['db']->status){
		$GLOBALS['db']->BeginTransaction();
		if($GLOBALS['db']->userExists($nuovoNome)){
			$GLOBALS['error_message'] = "Nome utente già utilizzato";
			$GLOBALS['db']->Rollback();
			return 0;
		}else{
			$res = $GLOBALS['db']->aggiungiUtente($nuovoNome,$pass);
			if($res)
				return $GLOBALS['db']->EndTransaction();
			else{
				$GLOBALS['db']->Rollback();
				$GLOBALS['error_message'] = "Errore del database";
				return $res;
			}
		}
	}else{
		$GLOBALS['error_message'] = "Errore nell'apertura del db";
		return 0;
	}

}

function prenotaAttivita(){
	if(!isset($_POST['cod_attivita']) || !isset($_POST['num_figli'])){
		$GLOBALS['error_message'] = "Il codice dell'attività o il numero figli non sono impostati";
		return 0;
	}

	$cod_attivita = test_input($_POST['cod_attivita']);
	$num_part = test_input($_POST['num_figli']);
	
	if($GLOBALS['db']->status){
		if(!is_numeric($num_part)){
			$GLOBALS['error_message'] = "Errore! Il dato non è numerico";
			return 0;
		}
		$num_part++;
		if($num_part > 4) {
			$GLOBALS['error_message'] = "Il numero massimo di figli è 3";
			return 0;
		}
		
		$GLOBALS['db']->BeginTransaction();

		$attivita = $GLOBALS['db']->CercaAttivita($cod_attivita);
		if (!$attivita) {
			$GLOBALS['error_message'] = "Errore! Attività non trovata";
			return 0;
		}

		if($attivita['posti_disponibili'] >= $num_part){
			$attivita['posti_disponibili'] -= $num_part;

			$attivita['posti_disponibili'] = ''.$attivita['posti_disponibili'];
			$num_part = ''.$num_part;
			//Let's update the db!!
			if(!($GLOBALS['db']->updateDisponibili($cod_attivita,$attivita['posti_disponibili']))){
				$GLOBALS['db']->Rollback();
				$GLOBALS['error_message'] = "Errore del database";
				return 0;
			}else{
				if(!($GLOBALS['db']->inserisciPrenotazione($_SESSION['s220318_user'],$cod_attivita,$num_part))){
					$GLOBALS['db']->Rollback();
					$GLOBALS['error_message'] .= "Impossibile completare la prenotazione";
					return 0;
				}
				else
					$GLOBALS['db']->EndTransaction();
					return 1;
			}
		}else{
			$GLOBALS['error_message'] = "Ci dispiace, non ci sono posti sufficienti";
			return 0;
		}
	}else{
		$GLOBALS['error_message'] = "Errore nell'apertura del db";
		return 0;
	}
}

function disdiciPrenotazione(){
	if(!isset($_POST['cod_attivita'])){
		$GLOBALS['error_message'] = "Il codice dell'attività da rimuovere non è settato";
		return 0;
	}
	$cod_attivita = test_input($_POST['cod_attivita']);

	if($GLOBALS['db']->status){
		$GLOBALS['db']->BeginTransaction();
		$res = $GLOBALS['db']->eliminaPrenotazione($_SESSION['s220318_user'], $cod_attivita);
		if($res){
			$GLOBALS['db']->EndTransaction();
			return 1;
		}else{
			$GLOBALS['error_message'] = "Non c'è una prenotazione per l'attività ".$cod_attivita." dell'utente ".$_SESSION['s220318_user'];
			$GLOBALS['db']->Rollback();
			return 0;		
		}
	}else{
		$GLOBALS['error_message'] = "Errore nell'apertura del db";
		return 0;
	}
}
?>