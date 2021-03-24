class Machine {
    constructor(socket, port) {
        this.port = port;
        this.socket = socket;
        this.state = {
            waitingOnOK: false,
        };

        this.socket.on('serialport:read', (data) => {
            data = data.trim();

            if(data === 'ok' && this.state.waitingOnOK) {
                console.log('waitingOnOK=false');

                this.state.waitingOnOK = false;
            }
        });
    }

    get waitingOnOK() {
        return Boolean(this.state.waitingOnOK);
    }

    receivedOK() {
        this.state.waitingOnOK = false;
    }

    waitOnOK() {
        this.state.waitingOnOK = true;
    }

    writeToCNC(gcode) {
        console.log('GCODE', gcode.replace(/\r\n|\r|\n/gm, ""));
        this.socket.emit('write', this.port, gcode);
    }
}

module.exports = Machine;
