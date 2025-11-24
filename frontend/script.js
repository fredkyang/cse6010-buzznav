document.addEventListener('DOMContentLoaded', () => {

    // --- 1. Initialize the Map ---

    // Define the boundaries for the GA Tech campus
    const southWest = L.latLng(33.770, -84.405);
    const northEast = L.latLng(33.783, -84.385);
    const campusBounds = L.latLngBounds(southWest, northEast);
    const paddedBounds = campusBounds.pad(0.02);

    const map = L.map('map', {
        center: [33.776, -84.397], // GA Tech Campus Center
        zoom: 16,
        minZoom: 15, // Set a minimum zoom
        maxBounds: paddedBounds // Restrict map panning
    });

    // Add the tile layer (OpenStreetMap)
    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
    }).addTo(map);

    // Store the route layer to clear it later
    let routeLayer = null;

    // --- 2. Get DOM Elements ---
    const navButton = document.getElementById('navigate-btn');
    const startInput = document.getElementById('start-building');
    const endInput = document.getElementById('end-building');
    const startList = document.getElementById('start-autocomplete-list');
    const endList = document.getElementById('end-autocomplete-list');

    const resultsPanel = document.getElementById('results-panel');
    const summaryEl = document.getElementById('route-summary');
    const stepsEl = document.getElementById('turn-by-turn-list');

    let buildingNames = [];

    // --- 3. Autocomplete Feature ---

    // Function to fetch the building list (Solves Problem 1)
    async function fetchBuildingList() {
        try {
            // This is now a REAL API call
            const response = await fetch('/api/buildings');
            if (!response.ok) {
                throw new Error('Failed to fetch building list');
            }
            const data = await response.json();
            buildingNames = data; // Use the real list
        } catch (error) {
            console.error(error);
            buildingNames = [];
        }
    }

    // Function to show/hide and filter the autocomplete list
    function setupAutocomplete(inputElement, listElement) {
        inputElement.addEventListener('input', () => {
            const value = inputElement.value.toLowerCase();
            listElement.innerHTML = ''; // Clear old items

            if (!value) return; // Don't show if empty

            const suggestions = buildingNames.filter(name =>
                name.toLowerCase().startsWith(value)
            );

            suggestions.forEach(name => {
                const item = document.createElement('div');
                item.textContent = name;
                item.addEventListener('click', () => {
                    inputElement.value = name; // Fill input with clicked name
                    listElement.innerHTML = ''; // Close list
                });
                listElement.appendChild(item);
            });
        });
    }

    // Close dropdowns if user clicks elsewhere
    document.addEventListener('click', (e) => {
        if (!e.target.closest('.autocomplete-container')) {
            startList.innerHTML = '';
            endList.innerHTML = '';
        }
    });

    // 4. Find Path Feature
    navButton.addEventListener('click', findPath);

    async function findPath() {
        const start = startInput.value;
        const end = endInput.value;

        if (!start || !end) {
            alert('Please enter a start and end location.');
            return;
        }

        // Clear old routes and show loading message
        if (routeLayer) {
            map.removeLayer(routeLayer);
        }
        resultsPanel.style.display = 'block';
        summaryEl.textContent = 'Calculating...';
        stepsEl.innerHTML = '';
        startList.innerHTML = '';
        endList.innerHTML = '';

        try {
            // API call
            const response = await fetch(`/api/navigate?start=${encodeURIComponent(start)}&end=${encodeURIComponent(end)}`);
            const data = await response.json();

            if (data.status !== "success") {
                // Handle errors from the C program
                throw new Error(data.message || 'Path not found.');
            }

            // Handle the returned data
            summaryEl.textContent = `Total Distance: ${data.total_distance.toFixed(1)} meters`;

            const instructionsHtml = data.instructions.map(step => `<p>${step}</p>`).join('');
            stepsEl.innerHTML = instructionsHtml;

            const pathPolyline = L.polyline(data.path_coordinates, {
                color: '#0058a0', // GATech Blue
                weight: 6
            });

            const startMarker = L.marker(data.path_coordinates[0]);
            const endMarker = L.marker(data.path_coordinates[data.path_coordinates.length - 1]);

            routeLayer = L.layerGroup([pathPolyline, startMarker, endMarker]);
            routeLayer.addTo(map);

            map.fitBounds(pathPolyline.getBounds(), { padding: [50, 50] });

        } catch (error) {
            resultsPanel.style.display = 'block';
            summaryEl.textContent = 'Failed to find path';
            stepsEl.innerHTML = `<p style="color: red;">${error.message}</p>`;
        }
    }

    // --- 5. Initial Setup ---
    fetchBuildingList(); // Fetch the real building list on page load
    setupAutocomplete(startInput, startList);
    setupAutocomplete(endInput, endList);
});