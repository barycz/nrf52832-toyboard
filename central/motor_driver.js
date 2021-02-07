
class MotorDriver {
	static Modes = Object.freeze({
		off: 0,
		coast: 1,
		break: 2,
		forward: 3,
		reverse: 4,
	});

	static FullDuty = 255;

	constructor(mode, duty, peripheral) {
		this.mode = mode;
		this.duty = duty;
		this.peripheral = peripheral;
	}

	get characteristicData() {
		let ret = new Uint8Array(2);
		ret[0] = this.mode;
		ret[1] = this.duty;
		return ret;
	}

	async turnOff() {
		this.mode = MotorDriver.Modes.off;
		this.duty = 0;
		return this.peripheral.writeDriverState();
	}

	async fullForward() {
		this.mode = MotorDriver.Modes.forward;
		this.duty = MotorDriver.FullDuty;
		return this.peripheral.writeDriverState();
	}

	async fullReverse() {
		this.mode = MotorDriver.Modes.reverse;
		this.duty = MotorDriver.FullDuty;
		return this.peripheral.writeDriverState();
	}

	async setDuty(duty) {
		this.duty = duty;
		return this.peripheral.writeDriverState();
	}

	async updateDuty(diff) {
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
		if (this.duty > MotorDriver.FullDuty) {
			this.duty = MotorDriver.FullDuty;
		}

		return this.peripheral.writeDriverState();
	}
}
