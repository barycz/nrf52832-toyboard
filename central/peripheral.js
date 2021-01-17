
log = console.log;

class Peripheral {
	static driverServiceUuid = '00000001-201b-286a-f29d-ff952dc319a2';
	static battServiceName = 'battery_service';

	async connect() {
		try {
			log('Requesting Bluetooth Device...');
			this.device = await navigator.bluetooth.requestDevice({
				filters: [{services: [Peripheral.driverServiceUuid, Peripheral.battServiceName]}]
			});

			log('Connecting to GATT Server...');
			this.server = await this.device.gatt.connect();

			log('Getting Services...');
			await Promise.all([
				this.initDriver(),
				this.initBatt(),
			]);

			log('Connected.');
		} catch(error) {
			log('exception ' + error);
		}
	}

	onBattLevelValueChanged(event) {
		let value = event.target.value;
		let battPercentage = value.getUint8(0);
		log('battery percentage ' + battPercentage);
	}

	async initDriver() {
		const characteristicUuid = '00000002-201b-286a-f29d-ff952dc319a2';

		this.driverService = await this.server.getPrimaryService(Peripheral.driverServiceUuid);
		this.driverModeChr = await this.driverService.getCharacteristic(characteristicUuid);
	}

	async initBatt() {
		this.battService = await this.server.getPrimaryService(Peripheral.battServiceName);
		this.battLevelChr = await this.battService.getCharacteristic('battery_level');
		await this.battLevelChr.startNotifications();
		let _this = this;
		this.battLevelChr.addEventListener('characteristicvaluechanged', function(event) {
			_this.onBattLevelValueChanged(event);
		});
	}

	async onReadClick() {
		try {
			log('Getting Characteristics value...');
			let characteristicValue = await this.driverModeChr.readValue();

			log('Characteristic: ' + characteristicValue.byteLength + ' bytes: ' + characteristicValue.getUint8());
		} catch(error) {
			log('exception ' + error);
		}
	}

	async writeDriverState(mode, duty) {
		try {
			let b = new Uint8Array(4);
			b[0] = mode;
			b[1] = duty;
			b[2] = mode;
			b[3] = duty;
			log('Writing ' + b);
			await this.driverModeChr.writeValueWithResponse(b);
		} catch(error) {
			log('exception ' + error);
		}
	}

	async turnOn() {
		await this.writeDriverState(3, 255);
	}

	async turnOff() {
		await this.writeDriverState(1, 0);
	}

	async blinkUpdate() {
		let states = [1, 5, 4, 0, 1, 5, 4, 0, 5, 0, 5, 0];
		await this.writeDriverState(states[this.blinkState]);
		if (++this.blinkState >= states.length) {
			this.blinkState = 0;
		}
		let _this = this;
		setTimeout(function() { _this.blinkUpdate(); }, 100);
	}

	startBlinking() {
		try {
			log('Start blinking ...');
			this.blinkState = 0;
			let _this = this;
			setTimeout(function() { _this.blinkUpdate(); });
		} catch(error) {
			log('exception ' + error);
		}
	}
}