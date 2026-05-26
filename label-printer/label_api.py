from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS
import os
from datetime import date

app = Flask(__name__, static_folder="dist")
CORS(app)  # Enable CORS for all routes and origins


def generate_label(capacity, internal_resistance):
    """
    Generate a label with the given capacity and internal resistance values.

    Args:
        capacity: Battery capacity value
        internal_resistance: Internal resistance value

    Returns:
        dict: Result of the label generation
    """
    try:
        # Import the make_label function
        from make_label import create_and_print_label

        # Call the label generation function
        result = create_and_print_label(capacity, internal_resistance)

        return {
            "success": True,
            "message": "Label generated and printed successfully",
            "data": result,
        }

    except Exception as e:
        return {
            "success": False,
            "message": f"Error generating label: {str(e)}",
            "data": None,
        }


@app.route("/", methods=["GET"])
def handle_label_request():
    """
    Handle GET requests to generate labels.
    Expected query args: ?capacity=2.6&internalResistance=0.09
    """
    try:
        # Get query parameters from request
        capacity = request.args.get("capacity")
        internal_resistance = request.args.get("internalResistance")

        # Extract required parameters
        if capacity is None:
            return (
                jsonify(
                    {
                        "success": False,
                        "message": "Missing required query parameter: capacity",
                    }
                ),
                400,
            )

        if internal_resistance is None:
            return (
                jsonify(
                    {
                        "success": False,
                        "message": "Missing required query parameter: internalResistance",
                    }
                ),
                400,
            )

        # Validate parameter types
        try:
            capacity = float(capacity)
            internal_resistance = float(internal_resistance)
        except (ValueError, TypeError):
            return (
                jsonify(
                    {
                        "success": False,
                        "message": "Invalid parameter types. capacity and internalResistance must be numbers",
                    }
                ),
                400,
            )

        # Generate the label
        result = generate_label(capacity, internal_resistance)

        # Return appropriate response
        if result["success"]:
            return jsonify(result), 200
        else:
            return jsonify(result), 500

    except Exception as e:
        return (
            jsonify({"success": False, "message": f"Internal server error: {str(e)}"}),
            500,
        )


@app.route("/health", methods=["GET"])
def health_check():
    """Health check endpoint"""
    return jsonify({"status": "healthy", "message": "Label API is running"}), 200


@app.route("/static", methods=["GET"])
def serve_static():
    """Serve static files from /dist folder"""
    # Check if dist folder exists
    if not os.path.exists("dist"):
        return (
            jsonify(
                {
                    "error": "Static files not found",
                    "message": "The /dist folder does not exist",
                }
            ),
            404,
        )

    # Try to serve index.html by default
    if os.path.exists("dist/index.html"):
        return send_from_directory("dist", "index.html")

    # If no index.html, list available files
    try:
        files = os.listdir("dist")
        return (
            jsonify(
                {
                    "message": "Static files available",
                    "files": files,
                    "note": "Add an index.html file to serve as the default page",
                }
            ),
            200,
        )
    except Exception as e:
        return jsonify({"error": "Could not read dist folder", "message": str(e)}), 500


@app.route("/static/<path:filename>")
def serve_static_file(filename):
    """Serve individual static files from /dist folder"""
    if not os.path.exists("dist"):
        return (
            jsonify(
                {
                    "error": "Static files not found",
                    "message": "The /dist folder does not exist",
                }
            ),
            404,
        )

    try:
        return send_from_directory("dist", filename)
    except FileNotFoundError:
        return (
            jsonify(
                {
                    "error": "File not found",
                    "message": f"File '{filename}' not found in /dist folder",
                }
            ),
            404,
        )


@app.route("/api", methods=["GET"])
def api_info():
    """API information endpoint"""
    return (
        jsonify(
            {
                "message": "Label Generation API",
                "endpoints": {
                    "GET /": "Generate label with capacity and internalResistance via query args",
                    "GET /static": "Serve static files from /dist folder",
                    "GET /static/<filename>": "Serve individual static files",
                    "GET /api": "API information (this endpoint)",
                    "GET /health": "Health check",
                },
                "example_request": {
                    "method": "GET",
                    "url": "/?capacity=2.6&internalResistance=0.09",
                },
            }
        ),
        200,
    )


if __name__ == "__main__":
    print("Starting Label Generation API...")
    print("API will be available at: http://127.0.0.1:5001")
    print("Static files will be served from: /dist folder")
    print("Example request:")
    print('curl "http://127.0.0.1:5001/?capacity=2.6&internalResistance=0.09"')

    app.run(host="0.0.0.0", port=5001, debug=True)
