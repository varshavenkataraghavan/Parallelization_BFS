
document.addEventListener('DOMContentLoaded', function () {
  const threadColors = [
    "#e6194B", "#3cb44b", "#ffe119", "#4363d8", "#f58231",
    "#911eb4", "#42d4f4", "#f032e6", "#fabed4", "#469990"
  ];

  let cy = null;

  function parseGraph(content) {
    const lines = content.trim().split("\n");
    const n = parseInt(lines[0]);
    const startNode = parseInt(lines[1]);
    const elements = [];

    for (let i = 0; i < n; i++) {
      elements.push({ data: { id: i.toString() } });
    }
    for (let i = 0; i < n; i++) {
      const row = lines[i + 2].split(" ").map(Number);
      for (let j = 0; j < n; j++) {
        if (row[j] === 1) {
          elements.push({ data: { id: `edge-${i}-${j}`, source: i.toString(), target: j.toString() } });
        }
      }
    }
    return { content, elements };
  }

  function uploadGraph(content, mode = "parallel") {
    const formData = new FormData();
    const blob = new Blob([content], { type: "text/plain" });
    formData.append("file", blob, "input.txt");
    formData.append("mode", mode);

    fetch("http://127.0.0.1:8000/upload", {
      method: "POST",
      body: formData,
    })
      .then((res) => res.json())
      .then((data) => {
        document.getElementById("output").textContent = data.output;
        if (data.thread_log) {
          const entries = data.thread_log.trim().split("\n");
          runAnimation(entries, mode);
        }
      })
      .catch((err) => {
        document.getElementById("output").textContent = `Error: ${err.message}`;
      });
  }

  function initializeCytoscape(elements) {
    if (cy) cy.destroy();
    cy = cytoscape({
      container: document.getElementById('cy'),
      elements: elements,
      style: [
        {
          selector: 'node',
          style: {
            'background-color': '#999',
            'label': 'data(id)',
            'text-valign': 'center',
            'text-halign': 'center',
            'color': '#fff',
            'font-size': 12,
            'width': 25,
            'height': 25
          }
        },
        {
          selector: 'edge',
          style: {
            'width': 1.5,
            'line-color': '#ccc',
            'curve-style': 'bezier'
          }
        }
      ],
      layout: { name: 'circle', padding: 50 }
    });
  }

  function runAnimation(logEntries, mode) {
    cy.nodes().style('background-color', '#999');
    const output = document.getElementById('output');
    output.textContent = '';

    logEntries.forEach((entry, idx) => {
      const match = entry.match(/Thread (\d+) visiting (\d+)/);
      if (match) {
        const threadId = parseInt(match[1]);
        const nodeId = match[2];
        setTimeout(() => {
          const node = cy.getElementById(nodeId);
          if (node) node.style('background-color', threadColors[threadId % threadColors.length]);
          output.textContent += entry + '\n';
          output.scrollTop = output.scrollHeight;
        }, idx * 200);
      }
    });
  }

  document.getElementById("graph-file").addEventListener("change", function () {
    const file = this.files[0];
    if (!file) return;

    const reader = new FileReader();
    reader.onload = function (e) {
      const content = e.target.result;
      const { content: rawContent, elements } = parseGraph(content);
      initializeCytoscape(elements);
      const mode = document.getElementById("mode-select").value;
      uploadGraph(rawContent, mode);
    };
    reader.readAsText(file);
  });
});
