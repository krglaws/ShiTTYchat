import time
from requests import get
import socket
import smtplib
from email.MIMEMultipart import MIMEMultipart
from email.MIMEText import MIMEText


EMAIL = ''
PASSWD = ''


def send_email(emailaddr, passwd, msg):

	# set up email
	email = MIMEMultipart()
	email['From'] = emailaddr
	email['To'] = emailaddr
	email['Subject'] = socket.gethostname() + ': Changed IP Notification'
	email.attach(MIMEText(msg, 'plain'))

	# connect to gmail account
	server = smtplib.SMTP('smtp.gmail.com', 587)
	server.starttls()
	server.login(emailaddr, passwd)

	# send
	server.sendmail(emailaddr, emailaddr, email.as_string())

	# done
	server.quit()


if __name__ == '__main__':
	currip = get('https://ipapi.co/ip/').text
	msg = 'Current IP: ' + currip
	send_email(EMAIL, PASSWD, msg)
	while(True):
		# check once every hour
		time.sleep(60**2)
		newip = get('https://ipapi.co/ip/').text
		if not newip == currip:
			currip = newip
			msg = 'New IP: ' + currip
			send_email(EMAIL, PASSWD, msg)
