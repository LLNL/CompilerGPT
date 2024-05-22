from openai import OpenAI
# ~ import openai
import json

client = OpenAI()

modellst = client.models.list()

print(modellst)

# ~ with open("query.json", "r") as f: msgarr = json.load(f)

#print(msgarr[1])

# ~ print("Ask our friend GPT:")
# ~ completion = client.chat.completions.create(
  # ~ model="gpt-3.5-turbo",
  # ~ messages=msgarr
# ~ )

# ~ print(completion.choices[0].message)
