
log = console.log;

class PeripheralConfig {
	static RawSize = 20;

	constructor(buffer) {
		this.rawData = new Uint8Array(buffer);
	}

	get rawColor() {
		return this.rawData.subarray(0, 3);
	}

	get rawName() {
		return this.rawData.subarray(4);
	}

	get color() {
		return '#' + Array.from(this.rawColor).map(
			c => c.toString(16).padStart(2, '0')
		).join('');
	}

	set color(colorStr) {
		const arr = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(colorStr).map(x => parseInt(x, 16));
		for (let i = 0; i < this.rawColor.length; i++) {
			this.rawColor[i] = i + 1 < arr.length ? arr[i + 1] : 0;
		}
	}

	get name() {
		return Array.from(this.rawName.filter(x => x != 0)).map(
			x => String.fromCharCode(x)
		).join('');
	}

	set name(nameStr) {
		for (let i = 0; i < this.rawName.length; i++) {
			this.rawName[i] = i < nameStr.length ? nameStr.charCodeAt(i) : 0;
		}
	}
}

class Peripheral {
	static driverServiceUuid = '00000001-201b-286a-f29d-ff952dc319a2';
	static battServiceName = 'battery_service';

	constructor() {
		this.motorDrivers = [
			new MotorDriver(MotorDriver.Modes.coast, 0, this),
			new MotorDriver(MotorDriver.Modes.coast, 0, this),
		];
		this.battPercentageChanged = new Signal;
	}

	async readConfig() {
		const rawData = await this.confChr.readValue();
		return new PeripheralConfig(rawData.buffer);
	}

	async writeConfig(config) {
		return this.confChr.writeValueWithResponse(config.rawData);
	}

	async requestDevice() {
		log('Requesting Bluetooth Device...');
		this.device = await navigator.bluetooth.requestDevice({
			filters: [{services: [Peripheral.driverServiceUuid, Peripheral.battServiceName]}]
		});
	}

	async connect() {
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
		//log('battery percentage ' + battPercentage);
		this.battPercentageChanged.emit(battPercentage);
	}

	async initDriver() {
		const modeCharacteristicUuid = '00000002-201b-286a-f29d-ff952dc319a2';
		const confCharacteristicUuid = '00000003-201b-286a-f29d-ff952dc319a2';

		this.driverService = await this.server.getPrimaryService(Peripheral.driverServiceUuid);
		this.driverModeChr = await this.driverService.getCharacteristic(modeCharacteristicUuid);
		this.confChr = await this.driverService.getCharacteristic(confCharacteristicUuid);
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
			driver.turnOff();
		});

		await this.writeDriverState();
	}
}