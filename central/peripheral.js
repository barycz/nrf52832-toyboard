
log = console.log;

class Peripheral {
	static driverServiceUuid = '00000001-201b-286a-f29d-ff952dc319a2';
	static battServiceName = 'battery_service';

	constructor() {
		this.motorDrivers = [
			new MotorDriver(MotorDriver.Modes.coast, 0),
			new MotorDriver(MotorDriver.Modes.coast, 0),
		];
		this.battPercentageChanged = new Signal;
	}

	async connect() {
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
	}

	onBattLevelValueChanged(event) {
		let value = event.target.value;
		let battPercentage = value.getUint8(0);
		log('battery percentage ' + battPercentage);
		this.battPercentageChanged.emit(battPercentage);
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

	async writeDriverState() {
		try {
			let b = this.motorDrivers.reduce((accum, driver) => {
				let driverData = driver.characteristicData;
				let ret = new Uint8Array(accum.length + driverData.length);
				ret.set(accum);
				ret.set(driverData, accum.length);
				return ret;
			}, new Uint8Array());
			log('Writing ' + b);
			await this.driverModeChr.writeValueWithResponse(b);
		} catch(error) {
			log('exception ' + error);
		}
	}

	async updateMotorDriver(id, fn) {
		fn(this.motorDrivers[id]);
		await this.writeDriverState();
	}

	async turnOff() {
		this.motorDrivers.forEach(driver => {
			driver.mode = MotorDriver.Modes.off;
			driver.duty = 0;
		});

		await this.writeDriverState();
	}
}