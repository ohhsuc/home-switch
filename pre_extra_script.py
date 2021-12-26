Import("env")

# access to global construction environment
print(env)

env.Execute("yarn && yarn build")
