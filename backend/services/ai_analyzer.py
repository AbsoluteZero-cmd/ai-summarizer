from transformers import AutoModelForCausalLM, AutoTokenizer

def load_and_run_model(prompt: str = None):

    model_name = "LGAI-EXAONE/EXAONE-4.0-1.2B"

    model = AutoModelForCausalLM.from_pretrained(
        model_name,
        torch_dtype="bfloat16",
        device_map="auto"
    )
    tokenizer = AutoTokenizer.from_pretrained(model_name)

    # choose your prompt
    if prompt is None:
        prompt = "Explain how wonderful you are"

    messages = [
        {"role": "user", "content": prompt}
    ]
    input_ids = tokenizer.apply_chat_template(
        messages,
        tokenize=True,
        add_generation_prompt=True,
        return_tensors="pt"
    )

    output = model.generate(
        input_ids.to(model.device),
        max_new_tokens=128,
        do_sample=False,
    )
    return tokenizer.decode(output[0])
