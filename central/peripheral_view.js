
class PeripheralView extends View {
	constructor(peripheral, parent) {
		super(parent);

		this.peripheral = peripheral;
		this.motorDriverViews = [];

		this.addButton("off", function() {
			peripheral.turnOff();
		});

		peripheral.motorDrivers.forEach(motor => {
			const motorDriverView = new MotorDriverView(motor, this.topDiv);
			this.motorDriverViews.push(motorDriverView);
		});

		let _this = this;
		this.batt = this.addElement("span");
		this.peripheral.battPercentageChanged.connect(percentage => {
			_this.batt.innerText = percentage;
		});

		this.parent.appendChild(this.topDiv);
	}
}