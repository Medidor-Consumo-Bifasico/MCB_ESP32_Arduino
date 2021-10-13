
/* Principal */
var connection = new WebSocket('ws://'+location.hostname+':81/');

connection.onmessage = function(event){
    
    var full_data = event.data;
    console.log(full_data);
    var data = JSON.parse(full_data);
    var opcion = data.OPCION;

    if (opcion == "CLAVES"){
        document.getElementById("refresh").style.backgroundColor = "#4091EC";
        var listSSID = data.SSID;
        var s1 = document.getElementById("ssd");
        s1.innerHTML = "";

        for(var option in listSSID)
        {
            var newoption = document.createElement("option");
            newoption.value = listSSID[option];
            newoption.innerHTML = listSSID[option];
            s1.options.add(newoption);
        }
        document.getElementById("refresh").style.backgroundColor = "gray";
    }

    else if (opcion == "PAGINA"){
        var resultado = data.STATUS;
        if (resultado == "OK"){
            location.href = "/index_ok.html";
        }
        else if (resultado == "FALLO"){
            location.href = "/index_error.html";
        }
    }
  }


function redes()
{
    var full_data = "{'OPCION':'update'}";
    connection.send(full_data);
}

function credencialesWIFI()
{
    var ssid = document.getElementById("ssd").value;
    var pass = document.getElementById("pass").value;

    if (ssid==""){
        alert("No se puede dejar el SSID vacio");
    }

    else{
        document.getElementById("Submit").style.backgroundColor = "gray";
        var full_data = '{"OPCION":"ssid", "SSID":"'+ssid+'","PASS":"'+pass+'"}';
        connection.send(full_data);
    }
}




var pass = false
function changePass()
{
    var s1 = document.getElementById("pass");
    if (pass){
        document.getElementById("mostrar").style.backgroundColor = "gray";        
        s1.type = "password";
    }
    else{
        document.getElementById("mostrar").style.backgroundColor = "#4091EC";
        s1.type = "text";
    }
    pass = !(pass)
    
}


/* ERROR */
function changePag(tipo){
    if (tipo == "error"){
        location.href = "/";
    }
    
}