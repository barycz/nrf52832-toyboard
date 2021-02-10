
class PeripheralView extends View {
	constructor(peripheral, parent) {
		super({"className":"PeripheralView"}, parent);

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

		this.colorInput = this.addElement("input", {type: "color"});
		this.nameInput = this.addElement("input", {type: "text"});
		this.peripheral.readConfig().then(config => {
			_this.colorInput.value = config.color;
			_this.nameInput.value = config.name;
		});
		this.addButton("Save", () => {
			const buffer = new ArrayBuffer(PeripheralConfig.RawSize);
			const config = new PeripheralConfig(buffer);
			config.name = _this.nameInput.value;
			config.color = _this.colorInput.value;
			_this.peripheral.writeConfig(config);
		});
	}
}