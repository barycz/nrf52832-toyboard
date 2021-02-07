
class Ctrl {
	constructor() {
		this.peripherals = [];
		this.peripheralViews = [];
		const _this = this;
		document.onkeydown = e => _this.onKeyDown(e);
	}

	async addPeripheral() {
		try {
			let p = new Peripheral();
			await p.connect();
			let v = new PeripheralView(p, document.getElementById("peripherals"));
			this.peripherals.push(p);
			this.peripheralViews.push(v);
		} catch(error) {
			console.log('addPeripheral failed ' + error);
		}
	}

	forEachChecked(f) {
		this.peripheralViews.forEach(p => p.motorDriverViews.forEach(dv => {
			if (dv.checked) {
				f(dv);
			}
		}));
	}

	turnOff() {
		this.peripherals.forEach(p => p.turnOff());
	}

	onKeyDown(e) {
		switch (e.code) {
			case "ArrowUp":
				this.forEachChecked(dv => dv.motor.updateDuty(20));
				break;
			case "ArrowDown":
				this.forEachChecked(dv => dv.motor.updateDuty(-20));
				break;
		}
	}
}

const ctrl = new Ctrl();
