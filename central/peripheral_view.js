
class PeripheralView {
	constructor(peripheral, parent) {
		this.peripheral = peripheral;
		this.parent = parent;
		this.topDiv = document.createElement("div");

		this.addButton("off", function() {
			peripheral.turnOff();
		});

		for (let id = 0; id < peripheral.motorDrivers.length; id++) {
			this.addButton("+", function() {
				peripheral.updateMotorDriver(id, motor => motor.updateDuty(50));
			});

			this.addButton("0", function() {
				peripheral.updateMotorDriver(id, motor => motor.duty = 0);
			});

			this.addButton("-", function() {
				peripheral.updateMotorDriver(id, motor => motor.updateDuty(-50));
			});
		}

		let _this = this;
		this.batt = this.addElement("span");
		this.peripheral.battPercentageChanged.connect(percentage => {
			_this.batt.innerText = percentage;
		});

		this.parent.appendChild(this.topDiv);
	}

	addButton(text, action) {
		let btn = this.addElement("button");
		btn.innerText = text;
		btn.onclick = action;
		return btn;
	}

	addElement(type) {
		let element = document.createElement(type);
		this.topDiv.appendChild(element);
		return element;
	}
}