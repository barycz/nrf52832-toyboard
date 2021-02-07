class MotorDriverView extends View {
	constructor(motor, parent) {
		super(parent);

		this.motor = motor;

		this.check = this.addElement("input", {type: "checkbox"});
		this.addButton("FF", () => motor.fullForward());
		this.addButton("+", () => motor.updateDuty(50));
		this.addButton("0", () => motor.setDuty(0));
		this.addButton("-", () => motor.updateDuty(-50));
		this.addButton("FR", () => motor.fullReverse());
	}

	get checked() {
		return this.check.checked;
	}
}