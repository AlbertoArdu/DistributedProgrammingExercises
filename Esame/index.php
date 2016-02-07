<?php

require "ClassDB.php";
require "functions.php";

HttpsAssert();
session_name("s220318_session");
session_start();

if(isset($_SESSION['s220318_user'])){
	$loggato = CheckConnTimeout();
}elseif(isset($_POST['action']) && $_POST['action']=='login'){
	$loggato = Login();
	if(!$loggato)
		$successo=0;
}else{
	$loggato=0;
}

if (isset($_REQUEST['action'])){
	if(!$loggato){
		switch ($_REQUEST['action']) {
			case 'registrazione':
				$in_registrazione = true;
				break;
			case 'registra':
				if(($successo = registraUtente())==1){
					$GLOBALS['message']="Utente registrato con successo";
				}
				break;
			default:
				break;
		}
	}else{
		switch ($_REQUEST['action']) {
			case 'prenAtt':
				$successo = prenotaAttivita();
				if($successo)
					$GLOBALS['message']="Attività prenotata con successo";
				break;
			case 'logout':
				Logout();
				break;
			case 'remAtt':
				$successo = disdiciPrenotazione();
				if($successo)
					$GLOBALS['message']="Prenotazione disdetta con successo";
				break;
			default:
				break;
		}
	}
}

?>
<!DOCTYPE html>
<html>
<head>
<title>Prenotazioni Attività</title>
<meta http-equiv="Content-type" content="text/html;charset=utf-8">
<META http-equiv="Content-Style-Type" content="text/css">
<meta description="Sito sviluppato per l'esame di programmazione distribuita I al politecnico di Torino">
<meta keywords="attivita attività sport prenotazione figli famiglia divertimento hobby svago">
<meta author="Alberto Ardusso s220318">
<link rel="icon" href="img/clicknrun.ico" type="image/x-icon">
<link rel="stylesheet" type="text/css" href = "stylesheets/stile.css">
<script type="text/javascript" src="script/jquery-1.11.3.min.js"></script>
<script type="text/javascript" src = "script/lib.js"></script>
</head>
<body>
<?php
	require "header.php";
?>
<div class="center">
<?php
	if (isset($in_registrazione)) {
		require 'registrazione.php';
	}else{
		require "menu.php";
		if (isset($successo)) {
			require "message.php";
		}else{
			require "main.php";
		}
	}
?>
</div>
<script type="text/javascript">
	$(document).ready(controllaCookie());
	$(document).ready(function () {
		$("#checkJS").show();
	});
</script>
</body>
</html>