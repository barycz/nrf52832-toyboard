
class Signal {
	constructor() {
		this.slots = [];
	}

	connect(slot) {
		this.slots.push(slot);
	}

	emit(args) {
		this.slots.forEach(slot => slot(args));
	}
}