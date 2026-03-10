const NodeHelper = require("node_helper");
const fs = require("fs");

module.exports = NodeHelper.create({
  socketNotificationReceived(notification, payload) {
    if (notification !== "READ_JSON_FILE") return;

    const filePath = payload.filePath;

    fs.readFile(filePath, "utf8", (err, content) => {
      if (err) {
        this.sendSocketNotification("JSON_FILE_DATA", {
          data: null,
          error: `Kann Datei nicht lesen:\n${filePath}\n\n${err.message}`
        });
        return;
      }

      try {
        const data = JSON.parse(content);
        this.sendSocketNotification("JSON_FILE_DATA", { data, error: null });
      } catch (e) {
        this.sendSocketNotification("JSON_FILE_DATA", {
          data: null,
          error: `JSON-Parse-Fehler in:\n${filePath}\n\n${e.message}\n\nInhalt (Auszug):\n${content.slice(0, 400)}`
        });
      }
    });
  }
});
