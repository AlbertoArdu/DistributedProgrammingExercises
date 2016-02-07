<div>
	<div class="box_generico box_centrato">
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
				<input type="submit" value="Registrati" class="button">
			</div>
			<input type="hidden" name="action" value="registra">
		</form>
		<a href="index.php"><button class="button">Home</button></a>		
		<div id="errorMessage" class="error"></div>
	</div>
</div>
