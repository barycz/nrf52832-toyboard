
class PeripheralView {
	constructor(peripheral, parent) {
		this.peripheral = peripheral;
		this.parent = parent;
		this.topDiv = document.createElement("div");

		this.addButton("off", function() {
			peripheral.turnOff();
		});

		this.addButton("+", function() {
			peripheral.updateMotorDriver(0, motor => motor.updateDuty(50));
		});

		this.addButton("-", function() {
			peripheral.updateMotorDriver(0, motor => motor.updateDuty(-50));
		});

		this.addButton("+", function() {
			peripheral.updateMotorDriver(1, motor => motor.updateDuty(50));
		});

		this.addButton("-", function() {
			peripheral.updateMotorDriver(1, motor => motor.updateDuty(-50));
		});

		this.parent.appendChild(this.topDiv);
	}

	addButton(text, action) {
		let btn = document.createElement("button");
		btn.innerHTML = text;
		btn.onclick = action;
		this.topDiv.appendChild(btn);
	}
}