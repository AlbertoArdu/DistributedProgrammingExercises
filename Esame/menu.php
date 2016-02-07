<div class="menu">
	<div id="checkJS" hidden>
		<div id="errorMessage" class="error"></div>
<?php
	if( !isset($loggato) || $loggato == 0){
?>

		<div id="boxLogin">
			<form action="index.php" method="POST" onsubmit="return controllaCampiLogin()">
			<div class="margin">
				<span class="label"><label for="user">Nome Utente</label></span>
				<span><input class="textbox" id="inputNome" name="user" type="text" onchange="pulisciErrore()"></span>
			</div>
			<div class="margin">
				<span class="label"><label for="pass">Password</label></span>
				<span><input class="textbox" id="inputPass" name="pass" type="password" onchange="pulisciErrore()"></span>
			</div>
			<div>
				<input type="submit" value="Login" class="button">
			</div>
			<input type="hidden" name="action" value="login">
			</form>
		</div>
		<div class="margin"><a href="index.php?action=registrazione">Non sei registrato?</a></div>

	<?php 
		}else{
			#pulsanti per:
			# - visualizzare le attività prenotate,
			# - poter rimuovere un'attività
	?>
		<div id="pulsantieraLat">
			<div id="formPrenota">
				<form action="index.php" method="POST" onsubmit="return controllaCampiPrenotazione()">
					<div>
						<input type="submit" id="submitPrenot" value="Prenota attivita" disabled="true" class="button">
					</div>
					<div id="campo_figli" hidden>
						<label for="num_figli" id="labelNumFigli">Numero Figli</label>
						<input type="text" class="textbox" id="numFigli" name="num_figli" onchange="pulisciErrore()">
					</div>
					<div id="campoCodice" hidden>
						<label for="cod_attivita" id="labelCodAtt"></label>
						<input type="text" class="textbox" id="codAttivita" name="cod_attivita" value="">
					</div>
					<input type="hidden" name="action" value="prenAtt">
				</form>
			</div>
			<div>
				<div>
					<button class="button" onclick="visualizzaAttivitaPrenotate()">Visualizza attività prenotate</button>
				</div>
				<div id="elencoAttPrenotate" class="box_generico" style="display:none;">
	<?php
			$tuple = $GLOBALS['db']->cercaPrenotazioni($_SESSION['s220318_user']);
			$s='';
			if($tuple){
				$s .= "<table id='tabAttPren'>";
				foreach ($tuple as $t) {
					$s .= "<tr>";
					$s .= "<td>".$t['nome']."(".$t['codice'].")<br>No partecipanti: ".$t['num_partecipanti']."</td>";
					$s .=" <td><img src='./img/delete.png' alt='Elimina prenotazione' class='rem_button' onclick='rimuoviPrenotazione(\"".$t['codice']."\")'></td>";
					$s .= "</tr>\n";
				}
				$s .= "</table>";
				echo $s;
			}else{
				echo "<div class='error'>Non hai ancora fatto nessuna prenotazione</div>";
			}
	?>
				</div>
			</div>
			<div>
				<form action="index.php" method="POST">
					<input type="hidden" name="action" value="logout">
					<input type="submit" value="Logout" class="button">
				</form>
			</div>
		</div>
		<form id="eliminaPren" action="index.php" method="POST" hidden>
			<input type="hidden" name="action" value="remAtt">
			<input type="hidden" id="codAttRem" name="cod_attivita" value="">
		</form>
		<?php
			}
		?>
	</div>
	<div>
		<noscript class="error">
			Attenzione! Attivare Javascript per poter usufruire di tutte le funzionalità del sito!
		</noscript>
	</div>
</div>