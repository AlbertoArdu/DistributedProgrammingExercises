
<?php
	$scelta_sport = array("Basket","Calcio","Nuoto","Volley","Sci");

 ?>
<!DOCTYPE html public "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta http-equiv="Content-type" content="text/html;charset=utf-8">
<link rel="stylesheet" type="text/css" href="stile.css">
	<title>Home</title>
</head>
<body .nav>
<?php 

if(!isset($_POST["esercizio"])){

 ?>
<div class="center">
<form action="prova.php" method="POST" >
	<input type="hidden" name="esercizio" value="6_1_1">
	<div><input name="in_text" id="in_text" type="text"></div>
	<span><input type="reset" value="Reset"></span>
	<span><input type="submit" value="Submit"></span>
</form>

<form action="prova.php" method="POST" >
	<input type="hidden" name="esercizio" value="6_1_2">
	<select name="number">
		<?php
			for ($i=1; $i <= 10; $i++) { 
				echo '<option value="'.$i.'">'.$i.'</option>';
			}
		 ?>

	</select>
	<span><input type="reset" value="Reset"></span>
	<span><input type="submit" value="Submit"></span>
</form>

<form action="prova.php" method="POST" >
	<input type="hidden" name="esercizio" value="6_1_3">
	<div><input type="text" name="nome"></div>
	<div><input type="text" name="cognome"></div>
	<select name="sport">
		<?php
			for ($i=0; $i < 5; $i++) { 
				echo '<option value="'.$scelta_sport[$i].'">'.$scelta_sport[$i].'</option>';
			}
		?>
		</select>
	<span><input type="reset" value="Reset"></span>
	<span><input type="submit" value="Submit"></span>
</form>
</div>
<?php 

} else{

 ?>
<div>
<?php 
	switch ($_POST["esercizio"]) {
		case '6_1_1':
			$testo=htmlentities($_POST["in_text"]);
			echo "L'utente ha inserito ".$testo;
		break;
		case '6_1_2':
			$n = intval($_POST["number"]);
			if($n > 0 && $n <= 10){
				echo "<table><tr><th>Numero</th><th>Quadrato</th><th>Cubo</th></tr>";

				for ($i=1; $i < $_POST["number"]; $i++) { 
					echo "<tr>";
					$quadrato = $i * $i;
					$cubo = $i * $i * $i;
					echo "<td>".$i."</td><td>".$quadrato."</td><td>".$cubo."</td>";
				}
				echo "</table>";
			}
		break;
		case '6_1_3':
			echo "<p>Esercizio 6.1.3</p>";
			if( isset($_POST['nome']) && isset($_POST['cognome']) && isset($_POST['sport']) ){
				echo "<table><tr><th>Chiave</th><th>Valore</th></tr>";
				foreach ($_POST as $key => $value) {

					$a = htmlentities($value);
					$b = htmlentities($key);
					echo "<tr><td>".$b."</td><td>".$a."</td></tr>";
				}
				foreach ($_SERVER as $key => $value) {
					$a = htmlentities($value);
					$b = htmlentities($key);
					echo "<tr><td>".$b."</td><td>".$a."</td></tr>";
				}
				echo "/table";
			}
		break;
		default:
			echo "<p>Abbiamo un problema</p>";
		break;
	}

 ?>
</div>
<?php } ?>
<div class="footer"><span class="logo"><img alt="Stemma polito" id="logo-poli" src="http://security.polito.it/img/polito.gif"></span><span class="testo">Autore: <span class="firma">Alberto Ardusso</span><span class="email">ardusso.alberto@gmail.com</span></span></div>
</body>
</html>