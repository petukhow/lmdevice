from flask import Flask, request, jsonify
import requests
import os

app = Flask(__name__)

ANTHROPIC_API_KEY = os.environ.get("ANTHROPIC_API_KEY")
ANTHROPIC_URL = "https://api.anthropic.com/v1/messages"
DEVICE_SECRET = os.environ.get("DEVICE_SECRET")

@app.route("/ask", methods=["POST"])
def ask():
    if request.headers.get("X-Device-Secret") != DEVICE_SECRET:
        return jsonify({"error": "unauthorized"}), 401

    data = request.get_json()
    user_message = data.get("message", "")

    if not user_message:
        return jsonify({"error": "no message provided"}), 400

    response = requests.post(
        ANTHROPIC_URL,
        headers={
            "x-api-key": ANTHROPIC_API_KEY,
            "anthropic-version": "2023-06-01",
            "content-type": "application/json",
        },
        json={
            "model": "claude-sonnet-5",
            "max_tokens": 1024,
            "messages": [{"role": "user", "content": user_message}],
        },
        timeout=30,
    )

    if response.status_code != 200:
        return jsonify({"error": "anthropic api error", "details": response.text}), 502

    result = response.json()
    reply_text = result["content"][0]["text"]

    return jsonify({"reply": reply_text})


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)