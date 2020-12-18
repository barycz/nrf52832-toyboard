
log = console.log;

ctrl = {};

async function onConnectClick() {
	let serviceUuid = "12345678-1234-5678-1234-56789abcdef0";
	let characteristicUuid = "13345678-1234-5678-1334-56789abcdef3";

	try {
		log('Requesting Bluetooth Device...');
		ctrl.device = await navigator.bluetooth.requestDevice({
				filters: [{services: [serviceUuid]}]});

		log('Connecting to GATT Server...');
		ctrl.server = await ctrl.device.gatt.connect();

		log('Getting Service...');
		ctrl.service = await ctrl.server.getPrimaryService(serviceUuid);

		log('Getting Characteristics...');
		ctrl.characteristic = await ctrl.service.getCharacteristic(characteristicUuid);

		log('Connected.');
	} catch(error) {
		log('exception ' + error);
	}
}

async function onReadClick() {
	try {
		log('Getting Characteristics value...');
		let characteristicValue = await ctrl.characteristic.readValue();

		log('Characteristic: ' + characteristicValue.byteLength + ' bytes: ' + characteristicValue.getInt32());
	} catch(error) {
		log('exception ' + error);
	}
}

async function onSetClick() {
	try {
		log('Writing one ...');
		let b = new Int32Array(1);
		b[0] = 0x12345678;
		await ctrl.characteristic.writeValueWithResponse(b);
	} catch(error) {
		log('exception ' + error);
	}
}

async function onResetClick() {
	try {
		log('Writing zero ...');
		let b = new Int32Array(1);
		b[0] = 0;
		await ctrl.characteristic.writeValueWithResponse(b);
	} catch(error) {
		log('exception ' + error);
	}
}