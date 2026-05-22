from docx import Document
import sys

try:
    doc = Document(r'f:\OS\Operating Systems, Module Practice.docx')
    for para in doc.paragraphs:
        if para.text.strip():
            print(para.text)
except Exception as e:
    print(f"Error: {e}")
