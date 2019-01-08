workflow "New workflow" {
  on = "push"
  resolves = [
    "Test within WinCE",
  ]
}

action "Test within WinCE" {
  uses = "navit-gps/Dockerfiles/wince@master"
  args = "echo blsa"
}
