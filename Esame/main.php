<div class="main">
<?php 
//IDEONA!!!
	// 1 - OnClick su questi blocchi si abilita il pulsante "prenota" e viene visualizzato un campo "numFigli"
	//		e viene inserito in un campo hidden il codice dell'attività;
	// 2 - onclick di prenota, se numFigli <= 3 -> POST al server per prenotare l'attività

	$elenco_attivita = $db->CercaAttivita();
	$html_boxes = "";
	if($elenco_attivita != 0){
		foreach ($elenco_attivita as $key => $value) {
			$html_boxes.='<div class="box_attivita';
			if ($loggato && $value['posti_disponibili']>0) {
				$html_boxes.= ' box_prenotabile" onclick="attivaPrenotazione(\''.$value['codice'].'\')';
			}
			$html_boxes.= '">';
			$html_boxes.= $value['nome']." (Codice: ".$value['codice'].")<br>";
			$html_boxes.= 'Posti rimasti '.$value['posti_disponibili'].'/'.$value['posti_totali'];
			$html_boxes.= '</div>';
		}
		echo $html_boxes;
	}	
?>
</div>