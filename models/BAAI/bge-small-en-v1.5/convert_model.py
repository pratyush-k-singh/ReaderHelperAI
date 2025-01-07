#!/usr/bin/env python3
import os
import sys
import torch
from transformers import AutoModel, AutoTokenizer
from pathlib import Path

def download_and_convert_model(model_name="BAAI/bge-small-en-v1.5", output_dir=None):
    """
    Downloads and converts the model to TorchScript format.
    """
    if output_dir is None:
        output_dir = Path(__file__).parent / "BAAI/bge-small-en-v1.5"
    
    print(f"Creating output directory: {output_dir}")
    os.makedirs(output_dir, exist_ok=True)

    print(f"Downloading model and tokenizer from {model_name}...")
    model = AutoModel.from_pretrained(model_name)
    tokenizer = AutoTokenizer.from_pretrained(model_name)

    print("Converting model to TorchScript format...")
    model.eval()
    
    # Create example input for tracing
    example_text = "This is an example text for tracing the model."
    inputs = tokenizer(example_text, return_tensors="pt")
    
    # Trace the model
    with torch.no_grad():
        traced_model = torch.jit.trace(
            model, 
            (inputs["input_ids"], inputs["attention_mask"])
        )
    
    # Save the converted model
    model_path = os.path.join(output_dir, "model.pt")
    torch.jit.save(traced_model, model_path)
    print(f"Saved converted model to {model_path}")

    # Save the tokenizer
    tokenizer_path = os.path.join(output_dir, "tokenizer.json")
    tokenizer.save_pretrained(output_dir)
    print(f"Saved tokenizer to {tokenizer_path}")

    # Verify the conversion
    print("\nVerifying conversion...")
    try:
        loaded_model = torch.jit.load(model_path)
        with torch.no_grad():
            test_output = loaded_model(inputs["input_ids"], inputs["attention_mask"])
        print("✅ Model verification successful!")
    except Exception as e:
        print(f"❌ Model verification failed: {e}")
        sys.exit(1)

if __name__ == "__main__":
    print("Starting model conversion process...")
    
    # Check for required packages
    try:
        import torch
        import transformers
    except ImportError:
        print("Required packages not found. Installing dependencies...")
        os.system("pip install torch transformers")
    
    try:
        download_and_convert_model()
    except Exception as e:
        print(f"Error during model conversion: {e}")
        sys.exit(1)