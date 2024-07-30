from openai import OpenAI
import json
#import os

#if 'REQUESTS_CA_BUNDLE' not in os.environ:
#  os.environ['REQUESTS_CA_BUNDLE'] = '/home/pirkelbauer2/certs/cspca.crt'

client = OpenAI()

with open("query.json", "r") as f: msgarr = json.load(f)

print(msgarr)

print("Ask our friend GPT:")
completion = client.chat.completions.create(
  model="gpt-4o",
  #~ model="gpt-4-turbo",
  #~ model="gpt-3.5-turbo",
  messages=msgarr
  #~ response_format={ "type" : "json_object" }
)


with open("response.txt", "w") as outf:
  print(completion.choices[0].message, file=outf)
  # ~ json.dump(completion.choices[0], outf)
