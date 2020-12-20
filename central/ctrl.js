
log = console.log;

ctrl = {};

async function onConnectClick() {
	let serviceUuid = "00000001-201b-286a-f29d-ff952dc319a2";
	let characteristicUuid = "00000002-201b-286a-f29d-ff952dc319a2";

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

		log('Characteristic: ' + characteristicValue.byteLength + ' bytes: ' + characteristicValue.getUint8());
	} catch(error) {
		log('exception ' + error);
	}
}

async function writeDriverState(s) {
	try {
		log('Writing ' + s);
		let b = new Uint8Array(1);
		b[0] = s;
		await ctrl.characteristic.writeValueWithResponse(b);
	} catch(error) {
		log('exception ' + error);
	}
}

async function onSetClick() {
	await writeDriverState(5);
}

async function onResetClick() {
	await writeDriverState(0);
}

async function blinkUpdate() {
	let states = [1, 5, 4, 0, 1, 5, 4, 0, 5, 0, 5, 0];
	await writeDriverState(states[ctrl.blinkState]);
	if (++ctrl.blinkState >= states.length) {
		ctrl.blinkState = 0;
	}
	setTimeout(blinkUpdate, 100);
}

function onBlinkClick() {
	try {
		log('Start blinking ...');
		ctrl.blinkState = 0;
		setTimeout(blinkUpdate);
	} catch(error) {
		log('exception ' + error);
	}
}