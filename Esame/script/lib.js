function controllaCookie(){
	document.cookie = "test=1234";

	var valore = document.cookie;
	valore = valore.split(";");
	if(valore.indexOf("test=1234") < 0){
		$("#boxLogin").html("I cookie non sono abilitati.<br>Non puoi fare il login");
	}
}

function attivaPrenotazione (codice) {
	var button_pren = document.getElementById("submitPrenot");
	var campo_figli = document.getElementById("campo_figli");
	var campo_cod = document.getElementById("campoCodice");
	var cod_attivita = document.getElementById("codAttivita");

	button_pren.disabled = false;
	campo_figli.hidden = false;
	campo_cod.hidden = false;
	cod_attivita.value = codice;

}

function controllaCampiPrenotazione () {
	if(!$("#numFigli").val()){
		$("#errorMessage").html('Il campo figli è richiesto');
		return false;
	}else if(isNaN($("#numFigli").val())){
		$("#errorMessage").html('Devi inserire un numero');
		return false;
	}else if (($("#numFigli").val()) > 3) {
		$("#errorMessage").html('Non si possono portare più di 3 figli');
		return false
	}
	return true;
}

function controllaCampiLogin () {
	if(!$("#inputNome").val()){
		$("#errorMessage").html('Devi inserire un nome utente');
		return false;
	}else if(!$("#inputPass").val()){
		$("#errorMessage").html('Devi inserire una password');
		return false
	}
	return true;
}

function pulisciErrore () {
	$("#errorMessage").html('');
}

function visualizzaAttivitaPrenotate() {
	$("#elencoAttPrenotate").slideToggle("slow");
}

function rimuoviPrenotazione (codice) {
	$("#codAttRem").val(codice);
	$("#eliminaPren").submit();
}