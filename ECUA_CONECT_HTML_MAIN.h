// HTML web page to NAME, PASS
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta http-equiv="X-UA-Compatible" content="ie=edge">
    <title>Formulario de contacto</title>
</head>

<style>
    @font-face {
  font-family: 'FontAwesome';
   font-weight: normal;
  font-style: normal;
}

*{
    margin: 0;
    padding: 0;
    box-sizing:border-box;
}

:focus{
    outline: none;
}

body{
    background: #DBDBDB;
    font-family: 'Open sans';
}

/* FORMULARIO =================================== */

.form_wrap{
    width: 900px;
    height: 350px;
    margin: 50px auto;
    display: flex;

    background: #fff;
    border-radius: 10px;
    overflow: hidden;
    box-shadow: 0px 0px 20px rgba(0, 0, 0, 0.2);
}

/* Informacion de Contacto*/

.cantact_info::before{
    content: '';
    width: 100%;
    height: 100%;

    position: absolute;
    top: 0;
    left: 0;

    background: #4091EC;
    opacity: 0.9;
}

.cantact_info{
    width: 37%;
    position: relative;

    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;

    background-size: cover;
    background-position: center center;

}

.info_title,
.info_items{
    position: relative;
    z-index: 2;
    color: #fff;
}

.info_title{
    margin-bottom: 60px;
}

.info_title span{
    font-size: 100px;
    display: block;
    text-align: center;
    margin-bottom: 15px;
}

.info_title h2{
    font-size: 30px;
    text-align: center;
}

.info_items p{
    display: flex;
    align-items: center;

    font-size: 21px;
    font-weight: 600;
    margin-bottom: 10px;
}

.info_items p:nth-child(1) span{
    font-size: 30px;
    margin-right: 10px;
}

.info_items p:nth-child(2) span{
    font-size: 50px;
    margin-right: 15px;
    margin-left: 4px;
}


/* Formulario de contacto*/
form.form_contact{
    width: 62%;
    padding: 30px 40px;
}

form.form_contact h2{
  font-size: 25px;
  font-weight: 600;
  color: #303030;
  margin-bottom: 30px;
}

form.form_contact .user_info{
    display: flex;
    flex-direction: column;
}

form.form_contact label{
    font-weight: 600;
}

form.form_contact input,
form.form_contact textarea{
    width: 100%;
    padding: 8px 0px 5px;
    margin-bottom: 20px;

    border: none;
    border-bottom: 2px solid #D1D1D1;

    font-family: 'Open sans';
  color: #5A5A5A;
  font-size: 14px;
  font-weight: 400;
}

form.form_contact textarea{
    max-width: 100%;
    min-width: 100%;
    max-height: 90px;
}

form.form_contact input[type="Submit"]{
    width: 180px;
  background: #4091EC;
  padding: 10px;
  border: none;
  border-radius: 25px;
    align-self: flex-end;
    color: #fff;
  font-size: 16px;
  font-weight: 600;
}
</style>

<body>
    <section class="form_wrap">
        <section class="cantact_info">
            <section class="info_title">
                <span class="fa fa-user-circle"></span>
                <h2>INFORMACION<br>DE CONTACTO</h2>
            </section>
            <section class="info_items">
                <p><span class="fa fa-envelope"></span> ecuaplus@gmail.com</p>
                <p><span class="fa fa-mobile"></span> +5939876543210</p>
            </section>
        </section>

        <form action="/get" class="form_contact">
            <h2></h2>
            <h2>CREDENCIALES WIFI</h2>
            <div class="user_info">
                    <label for="names">SSID: </label>
                    <input type="text" name="input1"><br>

                    <label for="email">PASSWORD: </label>
                    <input type="text" name="input2"><br>

                    <input type="Submit" value="Submit">
            </div>
        </form>
    </section>
</body>
</html>
)rawliteral";