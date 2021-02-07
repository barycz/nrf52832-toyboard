class View {
	constructor(parent) {
		this.topDiv = document.createElement("div");
		this.parent = parent;
		this.parent.appendChild(this.topDiv);
	}

	addButton(text, action) {
		let btn = this.addElement("button");
		btn.innerText = text;
		btn.onclick = action;
		return btn;
	}

	addElement(type, props) {
		let element = document.createElement(type);
		for (let p in props) {
			element[p] = props[p];
		}
		this.topDiv.appendChild(element);
		return element;
	}
}