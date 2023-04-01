const char MAIN_PAGE[] PROGMEM = R"=====( 
<!DOCTYPE html>
<html>
<head>
    <title>Kiekenkot</title>
    <meta charset="UTF-8">
    <meta http-equiv="Content-type" content="text/html; charset=UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <script src="https://cdn.jsdelivr.net/npm/chart.js@2.8.0"></script>    
    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/css/bootstrap.min.css" integrity="sha384-Gn5384xqQ1aoWXA+058RXPxPg6fy4IWvTNh0E263XmFcJlSAwiGgFAW/dAiS6JXm" crossorigin="anonymous">
    <link rel="stylesheet" href="style.css">
    <script src="scripts.js"></script>
</head>

<body onload="init()">
    <div class="container-fluid">
        <h1>Kiekenkot</h1>

        <div class="row">

            <div class="pb-3 col-sm-12 col-md-6 col-lg-4">
                <div class="card mb-3 px-0">
                    <div class="card-header font-weight-bold">
                        Deur is <span id="DoorState"></span>

                        <div class="btn-group" role="group pt-4">
                            <button type="button" class="btn btn-outline-light" onclick="processStatus('/changeDoor?doorState=open')">Openen</button>
                            <button type="button" class="btn btn-outline-light" onclick="processStatus('/changeDoor?doorState=close')">Sluiten</button>
                        </div>
                    </div>
                    <div class="card-body">
                        Tijd openen: <span id="OpenTime"></span><br>
                        Tijd sluiten: <span id="CloseTime"></span><br>
                    </div>
                </div>

                <div class="card px-3">
                    <div class="card-body">
                        Temperatuur: <span id="Temperature"></span> &nbsp;
                        &lsqb; <span id = "Min. Temperature"></span> &verbar;
                        <span id = "Max. Temperature"></span> &rsqb;
                        <br>
                        Luchtvochtigheid: <span id="Humidity"></span> &nbsp;
                        &lsqb; <span id = "Min. Humidity"></span> &verbar;
                        <span id = "Max. Humidity"></span> &rsqb;
                        <br>
                        CO<sub>2</sub> concentratie: <span id="CO2 Concentration"></span> ppm &nbsp;
                        &lsqb; <span id = "Min. CO2 Concentration"></span> &verbar;
                        <span id = "Max. CO2 Concentration"></span> &rsqb;
                        <br>
                    </div>
                </div>

            </div>

            <div class="pb-3 col-sm-12 col-md-6 col-lg-8">
                <div class="card px-3">
                    <canvas id="histogram"></canvas>
                </div>
            </div>


        </div>
    </div>
</body>
</html>
)====="; 