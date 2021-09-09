Import("env")

# access to global construction environment
print(env)

env.Execute("cd web && npm install && npm run build")
