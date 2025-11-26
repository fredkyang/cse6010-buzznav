document.addEventListener('DOMContentLoaded', () => {

    // --- 1. Initialize the Map ---
    const southWest = L.latLng(33.770, -84.405);
    const northEast = L.latLng(33.783, -84.385);
    const campusBounds = L.latLngBounds(southWest, northEast);
    const paddedBounds = campusBounds.pad(0.02);

    const map = L.map('map', {
        center: [33.776, -84.397],
        zoom: 16,
        minZoom: 15,
        maxBounds: paddedBounds
    });

    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
    }).addTo(map);

    let routeLayer = null;

    // --- 2. Get DOM Elements ---
    const navButton = document.getElementById('navigate-btn');
    const addViaBtn = document.getElementById('add-via-btn');
    const startInput = document.getElementById('start-building');
    const endInput = document.getElementById('end-building');
    const startList = document.getElementById('start-autocomplete-list');
    const endList = document.getElementById('end-autocomplete-list');
    const viaContainer = document.getElementById('via-points-container');

    const resultsPanel = document.getElementById('results-panel');
    const summaryEl = document.getElementById('route-summary');
    const stepsEl = document.getElementById('turn-by-turn-list');
    const optimizeCheckbox = document.getElementById('optimize-order-checkbox');

    let buildingNames = [];
    let viaPointCounter = 0;

    // --- 3. Via Points Management ---
    
    function addViaPoint() {
        viaPointCounter++;
        const viaId = `via-${viaPointCounter}`;
        const listId = `${viaId}-list`;

        const viaDiv = document.createElement('div');
        viaDiv.className = 'input-group autocomplete-container via-point-group';
        viaDiv.id = viaId;
        viaDiv.innerHTML = `
            <label for="${viaId}-input">Via Point:</label>
            <div style="display: flex; gap: 5px;">
                <input type="text" id="${viaId}-input" placeholder="e.g., Howey Building" autocomplete="off" style="flex: 1;">
                <button class="remove-via-btn" data-via-id="${viaId}">×</button>
            </div>
            <div class="autocomplete-items" id="${listId}"></div>
        `;

        viaContainer.appendChild(viaDiv);

        const viaInput = document.getElementById(`${viaId}-input`);
        const viaList = document.getElementById(listId);
        setupAutocomplete(viaInput, viaList);

        // Add remove button listener
        const removeBtn = viaDiv.querySelector('.remove-via-btn');
        removeBtn.addEventListener('click', () => {
            viaDiv.remove();
        });
    }

    function getViaPoints() {
        const viaInputs = viaContainer.querySelectorAll('input[type="text"]');
        const viaPoints = [];
        viaInputs.forEach(input => {
            if (input.value.trim()) {
                viaPoints.push(input.value.trim());
            }
        });
        return viaPoints;
    }

    addViaBtn.addEventListener('click', addViaPoint);

    // --- 4. Autocomplete Feature ---
    async function fetchBuildingList() {
        try {
            const response = await fetch('/api/buildings');
            if (!response.ok) {
                throw new Error('Failed to fetch building list');
            }
            const data = await response.json();
            buildingNames = data;
        } catch (error) {
            console.error(error);
            buildingNames = [];
        }
    }

    function setupAutocomplete(inputElement, listElement) {
        inputElement.addEventListener('input', () => {
            const value = inputElement.value.toLowerCase();
            listElement.innerHTML = '';

            if (!value) return;

            const suggestions = buildingNames.filter(name =>
                name.toLowerCase().startsWith(value)
            );

            suggestions.forEach(name => {
                const item = document.createElement('div');
                item.textContent = name;
                item.addEventListener('click', () => {
                    inputElement.value = name;
                    listElement.innerHTML = '';
                });
                listElement.appendChild(item);
            });
        });
    }

    document.addEventListener('click', (e) => {
        if (!e.target.closest('.autocomplete-container')) {
            startList.innerHTML = '';
            endList.innerHTML = '';
            document.querySelectorAll('.autocomplete-items').forEach(list => {
                list.innerHTML = '';
            });
        }
    });

    // --- 5. Find Path Feature ---
    navButton.addEventListener('click', findPath);

    async function findPath() {
        const start = startInput.value.trim();
        const end = endInput.value.trim();
        const viaPoints = getViaPoints();

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
        
        // Clear all autocomplete lists
        document.querySelectorAll('.autocomplete-items').forEach(list => {
            list.innerHTML = '';
        });

        try {
            const optimizeOrder = optimizeCheckbox.checked;

            // Build API URL
            let apiUrl;
            
            if (optimizeOrder) {
                // TSP mode - send all buildings without order
                apiUrl = `/api/navigate-tsp?`;
                const allBuildings = [start, ...viaPoints, end];
                allBuildings.forEach((building, index) => {
                    apiUrl += `building${index + 1}=${encodeURIComponent(building)}`;
                    if (index < allBuildings.length - 1) apiUrl += '&';
                });
            } else {
                // Normal mode - maintain order
                apiUrl = `/api/navigate?start=${encodeURIComponent(start)}`;
                viaPoints.forEach(via => {
                    apiUrl += `&via=${encodeURIComponent(via)}`;
                });
                apiUrl += `&end=${encodeURIComponent(end)}`;
            }

            const response = await fetch(apiUrl);
            const data = await response.json();

            if (data.status !== "success") {
                throw new Error(data.message || 'Path not found.');
            }

            // Handle the returned data
            summaryEl.textContent = `Total Distance: ${data.total_distance.toFixed(1)} meters`;

            // Format instructions with numbered steps
            const instructionsHtml = data.instructions.map((step, index) => {
                return `<div class="instruction-step">
                    <span class="step-number">${index + 1}</span>
                    <span class="step-text">${step}</span>
                </div>`;
            }).join('');
            stepsEl.innerHTML = instructionsHtml;

            const pathPolyline = L.polyline(data.path_coordinates, {
                color: '#0058a0',
                weight: 6
            });

            // Create markers
            const markers = [];
            
            // alert(data.path_coordinates)

            // Green marker for START
            const startMarker = L.marker(data.path_coordinates[0], {
                title: 'Start',
                icon: L.icon({
                    iconUrl: 'https://raw.githubusercontent.com/pointhi/leaflet-color-markers/master/img/marker-icon-2x-green.png',
                    shadowUrl: 'https://cdnjs.cloudflare.com/ajax/libs/leaflet/0.7.7/images/marker-shadow.png',
                    iconSize: [25, 41],
                    iconAnchor: [12, 41],
                    popupAnchor: [1, -34],
                    shadowSize: [41, 41]
                })
            }).bindPopup(`<b>Start:</b> ${start}`);
            markers.push(startMarker);
            
            alert(data.via_point_indices)
            alert(viaPoints)
            // Orange markers for VIA POINTS
            if (viaPoints.length > 0 && data.via_point_indices) {
                data.via_point_indices.forEach((index, i) => {
                    if (index >= 0 && index < data.path_coordinates.length) {
                        const viaMarker = L.marker(data.path_coordinates[index], {
                            title: `Via Point`,
                            icon: L.icon({
                                iconUrl: 'https://raw.githubusercontent.com/pointhi/leaflet-color-markers/master/img/marker-icon-2x-orange.png',
                                shadowUrl: 'https://cdnjs.cloudflare.com/ajax/libs/leaflet/0.7.7/images/marker-shadow.png',
                                iconSize: [25, 41],
                                iconAnchor: [12, 41],
                                popupAnchor: [1, -34],
                                shadowSize: [41, 41]
                            })
                        }).bindPopup(`<b>Via Point:</b> ${viaPoints[i]}`);
                        markers.push(viaMarker);
                    }
                });
            }

            // Red marker for END
            const endMarker = L.marker(data.path_coordinates[data.path_coordinates.length - 1], {
                title: 'Destination',
                icon: L.icon({
                    iconUrl: 'https://raw.githubusercontent.com/pointhi/leaflet-color-markers/master/img/marker-icon-2x-red.png',
                    shadowUrl: 'https://cdnjs.cloudflare.com/ajax/libs/leaflet/0.7.7/images/marker-shadow.png',
                    iconSize: [25, 41],
                    iconAnchor: [12, 41],
                    popupAnchor: [1, -34],
                    shadowSize: [41, 41]
                })
            }).bindPopup(`<b>Destination:</b> ${end}`);
            markers.push(endMarker);

            routeLayer = L.layerGroup([pathPolyline, ...markers]);
            routeLayer.addTo(map);

            map.fitBounds(pathPolyline.getBounds(), { padding: [50, 50] });

        } catch (error) {
            resultsPanel.style.display = 'block';
            summaryEl.textContent = 'Failed to find path';
            stepsEl.innerHTML = `<p style="color: red;">${error.message}</p>`;
        }
    }

    // --- 6. Initial Setup ---
    fetchBuildingList();
    setupAutocomplete(startInput, startList);
    setupAutocomplete(endInput, endList);
});