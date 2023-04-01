const char SCRIPTS_JS[] PROGMEM = R"=====(

var chart;

function init() {
    setInterval(function () { processStatus('/status'); }, 10000);
    processStatus('/status');

    chart = new Chart(document.getElementById("histogram"), {
        type: 'bar',
        data: {
            labels: ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23"]
        }
    });
}

function localDateTime(dateString, includedate) {
    var theDate = new Date(dateString);
    if (includedate) return theDate.toLocaleString();
    else return theDate.toLocaleTimeString();
}

function comparePassage(a, b) {
    if (a.tagID < b.tagID) { return -1; }
    if (b.tagID < a.tagID) { return 1; }
    if (a.timestamp > b.timestamp) { return -1; }
    if (b.timestamp > a.timestamp) { return 1; }
    return 0;
}

function processValues(xmlNodes) {
    const CELSIUS_TO_KELVIN = 273.15;
    const translate_doorstate_NL = { open: "open", closed: "gesloten" };

    for (var i = 0; i < xmlNodes.length; i++) {
        var statusitem = xmlNodes[i];
        var name = statusitem.getAttribute('name');
        var type = statusitem.getAttribute('type');
        var value = statusitem.innerHTML;

        switch (type) {
            case 'text/doorstate':
                value = translate_doorstate_NL[value];
                break;
            case 'time/UTC':
                value = localDateTime(value, false);
                break;
            case 'temperature/kelvin':
                value = (Number(value) - CELSIUS_TO_KELVIN).toFixed(1) + '&#176;C';
                break;
            case 'relative_humidity/percentage':
                value = Number(value).toFixed(1) + '&#37;';
                break;
            case 'co2/ppm':
                value = Number(value).toFixed(0);
                break;
            default:
        }
        try {
            var element = document.getElementById(name);
            element.innerHTML = value;

            if (statusitem.hasAttribute('timestamp')) {
                var timestamp = statusitem.getAttribute('timestamp');
                element.setAttribute("title", localDateTime(timestamp, true));
            }
        }
        catch { }
    }
}

function processHistogram(xmlNodes) {
            // process traffic node
            var passages = [];
            var passage;
            var N = xmlNodes.length;
            var i, j;

            for (i = 0; i < N; i++) {
                var passageitem = xmlNodes[i];
                passage = { tagID: passageitem.getAttribute('id'), timestamp: passageitem.getAttribute('time') };
                passages.push(passage);
            }
            passages.sort(comparePassage);

            var bins = [];
            var currentID = "";
            chart.data.datasets = [];

            function addDataset(id)
            {
                var legend = {
                    '3456559': { label: "test", colour: "#33cc33" },
                    '3448922': { label: "Bruno", colour: "#ff9900" },
                    '3467388': { label: "Grijsje", colour: "#666666" },
                    '3433363': { label: "Zwartje", colour: "#000000" }
                };
    
                var dataset = {
                    label: legend[id]['label'],
                    backgroundColor: legend[id]['colour'],
                    data: bins[id]
                };
                chart.data.datasets.push(dataset);
            }
            
            for (i = 0; i < N; i++) {
                passage = passages[i];
                if (passage.tagID != currentID) {

                    if (currentID != "") {
                        addDataset(currentID);
                    }

                    currentID = passage.tagID;
                    bins[currentID] = [];

                    for (j = 0; j < 24; j++) {
                        bins[currentID][j] = 0;
                    }
                }
                var hour = new Date(passage.timestamp).getHours();
                bins[currentID][hour]++;
            }

            chart.update({ duration: 0 });
}


function processStatus(URL) {

    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {

            // process status nodes
            processHistogram(this.responseXML.getElementsByTagName('traffic')[0].getElementsByTagName('passage'));
            processValues(this.responseXML.getElementsByTagName('status'));

        }
    };

    xhttp.open("GET", URL, true);
    xhttp.send();
}
)=====";