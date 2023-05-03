import subprocess
import sys
import signal
import time
import os
import pprint
import json

from contextlib import contextmanager
from subprocess import Popen, PIPE, STDOUT
from os import path
from time import sleep

# default port for the  server
port = "12345"

# default IP for the server
ip = "127.0.0.1"

# default UDP client path
udp_client_path = "udp_client"

# default size of test output line
test_output_line_size = 40

####### Process utils#######
class Process:
  """Class that represents a process which can be controlled."""

  def __init__(self, command, cwd=""):
    self.command = command
    self.started = False
    self.cwd = cwd

  def start(self):
    """Starts the process."""
    try:
      if self.cwd == "":
        self.proc = Popen(self.command, universal_newlines=True, stdin=PIPE, stdout=PIPE, stderr=PIPE)
      else:
        self.proc = Popen(self.command, universal_newlines=True, stdin=PIPE, stdout=PIPE, stderr=PIPE, cwd=self.cwd)
      self.started = True
    except FileNotFoundError as e:
      print(e)
      quit()

  def finish(self):
    """Terminates the process and waits for it to finish."""
    if self.started:
      self.proc.terminate()
      self.proc.wait(timeout=1)
      self.started = False

  def send_input(self, proc_in):
    """Sends input and a newline to the process."""
    if self.started:
      self.proc.stdin.write(proc_in + "\n")
      self.proc.stdin.flush()

  def get_output(self):
    """Gets one line of output from the process."""
    if self.started:
      return self.proc.stdout.readline()
    else:
      return ""

  def get_output_timeout(self, tout):
    """Tries to get one line of output from the process with a timeout."""
    if self.started:
      with timeout(tout):
        try:
          return self.proc.stdout.readline()
        except TimeoutError as e:
          return "timeout"
    else:
      return ""

  def get_error(self):
    """Gets one line of stderr from the process."""
    if self.started:
      return self.proc.stderr.readline()
    else:
      return ""

  def get_error_timeout(self, tout):
    """Tries to get one line of stderr from the process with a timeout."""
    if self.started:
      with timeout(tout):
        try:
          return self.proc.stderr.readline()
        except TimeoutError as e:
          return "timeout"
    else:
      return ""

  def is_alive(self):
    """Checks if the process is alive."""
    if self.started:
      return self.proc.poll() is None
    else:
      return False

####### Helper functions #######
@contextmanager
def timeout(time):
  """Raises a TimeoutError after a duration specified in seconds."""
  signal.signal(signal.SIGALRM, raise_timeout)
  signal.alarm(time)

  try:
    yield
  except TimeoutError:
    pass
  finally:
    signal.signal(signal.SIGALRM, signal.SIG_IGN)

def raise_timeout(signum, frame):
  """Raises a TimeoutError."""
  raise TimeoutError

def make_target(target):
  """Runs a makefile for a given target."""
  subprocess.run(["make " + target], shell=True)
  return path.exists(target)

def make_clean():
  """Runs the clean target in a makefile."""
  subprocess.run(["make clean"], shell=True)

def exit_if_condition(condition, message):
  """Exits and prints the test results if a condition is true."""
  if condition:
    print(message)
    make_clean()
    print_test_results()
    quit()

def get_procfs_values(rmem):
  """Reads TCP buffer sizes from procfs."""
  path = "/proc/sys/net/ipv4/tcp_" + ("rmem" if rmem else "wmem")
  #path = "tcp_" + ("rmem" if rmem else "wmem")
  file = open(path, "r")
  values = file.readline().split()
  if len(values) < 3:
    print("Error: could not read correctly from procfs")
    return ["error"]

  return values

def set_procfs_values(rmem, values):
  """Writes TCP buffer sizes to procfs."""
  path = "/proc/sys/net/ipv4/tcp_" + ("rmem" if rmem else "wmem")
  #path = "tcp_" + ("rmem" if rmem else "wmem")

  if not os.access(path, os.W_OK):
    print("Error: not enough permissions to write to procfs")
    return False

  file = open(path, "w")
  file.write(values[0] + " " + values[1] + " " + values[2])

  return True



def run_udp_client(mode=True, type="0"):
  """Runs a UDP client which generates messages on one or multiple topics."""
  if mode:
    udpcl = Process(["python3", "udp_client.py", ip, port], udp_client_path)
    udpcl.start()
    for i in range(19):
      outudp = udpcl.get_output_timeout(1)
    udpcl.finish()
  else:
    udpcl = Process(["python3", "udp_client.py", "--mode", "manual", ip, port], udp_client_path)
    udpcl.start()
    sleep(1)
    udpcl.send_input(type)
    sleep(1)
    udpcl.send_input("exit")
    udpcl.finish()


run_udp_client()
