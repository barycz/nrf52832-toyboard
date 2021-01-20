
class MotorDriver {
	static Modes = Object.freeze({
		off: 0,
		coast: 1,
		break: 2,
		forward: 3,
		reverse: 4,
	});

	constructor(mode, duty) {
		this.mode = mode;
		this.duty = duty;
	}

	get characteristicData() {
		let ret = new Uint8Array(2);
		ret[0] = this.mode;
		ret[1] = this.duty;
		return ret;
	}

	updateDuty(diff) {
		if (this.mode != MotorDriver.Modes.forward
			&& this.mode != MotorDriver.Modes.reverse) {
			this.mode = MotorDriver.Modes.forward;
		}
		if (this.mode == MotorDriver.Modes.reverse) {
			this.duty -= diff;
		} else {
			this.duty += diff;
		}
		if (this.duty < 0) {
			this.duty = -this.duty;
			if (this.mode == MotorDriver.Modes.forward) {
				this.mode = MotorDriver.Modes.reverse;
			} else {
				this.mode = MotorDriver.Modes.forward;
			}
		}
		if (this.duty > 255) {
			this.duty = 255;
		}
	}
}
