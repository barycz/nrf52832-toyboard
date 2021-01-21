
class Ctrl {
	constructor() {
		this.peripherals = [];
	}

	async addPeripheral() {
		try {
			let p = new Peripheral();
			await p.connect();
			let v = new PeripheralView(p, document.getElementById("peripherals"));
			this.peripherals.push(p);
		} catch(error) {
			console.log('addPeripheral failed ' + error);
		}
	}

	turnOff() {
		this.peripherals.forEach(p => p.turnOff());
	}
}

const ctrl = new Ctrl();
