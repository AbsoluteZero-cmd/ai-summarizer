def analyze_document(text_content: str) -> dict:
    clean_text = text_content.strip()[:2000]

    return {
        "summary": f"Analyzed {len(clean_text)} characters.",
        "keywords": ["React", "FastAPI", "Cloud"],
        "sentiment": "Neutral"
    }