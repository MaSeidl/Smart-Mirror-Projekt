/* MagicMirror²: MMM-JsonFile
 * Liest eine lokale JSON-Datei und zeigt sie als Tabelle an.
 */

Module.register("MMM-JsonFile", {
  defaults: {
    filePath: "/home/fsmt2-frontend/MagicMirror/modules/MMM-JsonFile/Sensordaten.Json",
    updateInterval: 2000,   // ms
    title: "Sensorwerte",
    sortKeys: true,
    showNested: true        // nested objects flach darstellen (z.B. room.temp)
  },

  start() {
    this.dataObj = null;
    this.error = null;

    this._read();
    setInterval(() => this._read(), this.config.updateInterval);
  },

  _read() {
    this.sendSocketNotification("READ_JSON_FILE", {
      filePath: this.config.filePath
    });
  },

  socketNotificationReceived(notification, payload) {
    if (notification === "JSON_FILE_DATA") {
      this.dataObj = payload.data;
      this.error = payload.error || null;
      this.updateDom();
    }
  },

  getDom() {
    const wrapper = document.createElement("div");

    if (this.config.title) {
      const h = document.createElement("div");
      h.className = "bright";
      h.innerText = this.config.title;
      wrapper.appendChild(h);
    }

    if (this.error) {
      const err = document.createElement("div");
      err.className = "small dimmed";
      err.style.whiteSpace = "pre-wrap";
      err.innerText = this.error;
      wrapper.appendChild(err);
      return wrapper;
    }

    if (!this.dataObj) {
      const loading = document.createElement("div");
      loading.className = "small dimmed";
      loading.innerText = "Lade JSON…";
      wrapper.appendChild(loading);
      return wrapper;
    }

    const flat = this.config.showNested ? flatten(this.dataObj) : this.dataObj;

    const keys = Object.keys(flat);
    if (this.config.sortKeys) keys.sort((a, b) => a.localeCompare(b));

    const table = document.createElement("table");
    table.className = "small";

    keys.forEach((k) => {
      const tr = document.createElement("tr");

      const tdKey = document.createElement("td");
      tdKey.className = "dimmed";
      tdKey.style.paddingRight = "12px";
      tdKey.innerText = k;

      const tdVal = document.createElement("td");
      tdVal.className = "bright";
      tdVal.innerText = formatValue(flat[k]);

      tr.appendChild(tdKey);
      tr.appendChild(tdVal);
      table.appendChild(tr);
    });

    wrapper.appendChild(table);
    return wrapper;

    function formatValue(v) {
      if (v === null || v === undefined) return "-";
      if (typeof v === "number") return String(v);
      if (typeof v === "boolean") return v ? "true" : "false";
      if (typeof v === "string") return v;
      // arrays/objects als JSON-String kurz darstellen
      return JSON.stringify(v);
    }

    function flatten(obj, prefix = "", out = {}) {
      if (obj === null || obj === undefined) return out;

      if (Array.isArray(obj)) {
        obj.forEach((val, idx) => {
          const key = prefix ? `${prefix}[${idx}]` : `[${idx}]`;
          if (val && typeof val === "object") flatten(val, key, out);
          else out[key] = val;
        });
        return out;
      }

      if (typeof obj === "object") {
        Object.keys(obj).forEach((key) => {
          const val = obj[key];
          const newKey = prefix ? `${prefix}.${key}` : key;
          if (val && typeof val === "object") flatten(val, newKey, out);
          else out[newKey] = val;
        });
      } else {
        out[prefix || "value"] = obj;
      }
      return out;
    }
  }
});
