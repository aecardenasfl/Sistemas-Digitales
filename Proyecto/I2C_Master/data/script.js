(() => {
    const gateway = `ws://${window.location.hostname}/ws`;
    let websocket;
    const reconnectTimeout = 2000;  // 2 seconds delay for reconnections
    let websocketOpen = false;       // Track WebSocket connection status

    // Init WebSocket and button when the page loads
    window.addEventListener('load', onload);

    function onload(event) {
        initWebSocket();  // Initialize WebSocket
        initButton();     // Initialize button after page loads
    }

    // Function to initialize the WebSocket connection
    function initWebSocket() {
        if (websocketOpen) {
            console.log('WebSocket connection already open or being established...');
            return;  // If already connected or attempting to connect, do nothing
        }
        console.log('Trying to open a WebSocket connection…');
        websocket = new WebSocket(gateway);

        // Define WebSocket event handlers
        websocket.onopen = onOpen;
        websocket.onclose = onClose;
        websocket.onmessage = onMessage;
        websocket.onerror = onError; // Handle errors
    }

    // Event handler for when the WebSocket connection opens
    function onOpen(event) {
        console.log('WebSocket connection opened');
        websocketOpen = true;  // Mark WebSocket as open
        getReadings();         // Request sensor readings from the server
    }

    // Send a message to the ESP32 server to get the readings
    function getReadings() {
        if (websocketOpen) {
            if (websocket.readyState === WebSocket.OPEN) {
                websocket.send("getReadings");
            } else {
                console.log("WebSocket is not open, cannot send getReadings.");
            }
        } else {
            console.log("WebSocket is not connected, cannot get readings.");
        }
    }

    // Event handler for when the WebSocket connection closes
    function onClose(event) {
        console.log('WebSocket connection closed');
        websocketOpen = false;  // Mark WebSocket as closed

        // Attempt to reconnect after 2 seconds
        setTimeout(() => {
            console.log('Attempting to reconnect...');
            initWebSocket();
        }, reconnectTimeout);
    }

    // Event handler for incoming messages from the server
    function onMessage(event) {
        console.log("Message received:", event.data);
        if (event.data === "Movimiento Detectado") {
            const movimientoElement = document.getElementById('movimiento');
            if (movimientoElement) {
                movimientoElement.innerHTML = "MOVIMIENTO DETECTADO";
                movimientoElement.style.display = 'block';  // Show the element

                // Hide the element after 5 seconds
                setTimeout(() => {
                    movimientoElement.style.display = 'none';
                }, 5000);
            } else {
                console.warn("Element with id 'movimiento' not found.");
            }
        } else {
            try {
                const myObj = JSON.parse(event.data);
                const keys = Object.keys(myObj);

                // Update HTML elements with sensor readings
                keys.forEach(key => {
                    const element = document.getElementById(key);
                    if (element) {
                        element.innerHTML = myObj[key];
                    } else {
                        console.warn(`Element with id '${key}' not found.`);
                    }
                });
            } catch (error) {
                console.error("Error parsing message:", error);
            }
        }
    }

    // Event handler for WebSocket errors
    function onError(event) {
        console.error('WebSocket error observed:', event);
    }

    // Initialize button and set up its event
    function initButton() {
        const button = document.getElementById('myButton');
        if (button) {
            button.addEventListener('click', () => {
                if (websocketOpen) {
                    if (websocket.readyState === WebSocket.OPEN) {
                        console.log('Button pressed, sending signal to ESP32');
                        websocket.send("botonPresionado");
                    } else {
                        console.log('WebSocket is not open, cannot send button signal.');
                    }
                } else {
                    console.log('WebSocket is not connected, cannot send button signal.');
                }
            });
        } else {
            console.warn('Button element not found on the page.');
        }
    }
})();