
png = ''
with open('www/balls.png', 'r') as file:
  png = file.read()

with open('www/balls2.png', 'w') as file:
  file.write(png)

