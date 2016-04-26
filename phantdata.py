import sys
from phant import Phant
from twilio.rest import TwilioRestClient
from time import sleep
 
# Your Account Sid and Auth Token from twilio.com/user/account
account_sid = "AC70e3378bc320d80f6d7de6f8c621cc91"
auth_token  = "af1003842796fd42560d7dae9565ed88"
client = TwilioRestClient(account_sid, auth_token)

p = Phant(public_key='q5JMKnDJKXCMnjbYr0lG', fields=['temp'], private_key='BVBdMRDBMAt60G1J8WV5')

while(True):
	data = p.get()
	print("Latest Loudness Value is: {}".format(data[0]['temp']))
	if float(data[0]['temp']) > 1500:
		message = client.messages.create(body="Loudness: {0}\nD313 Room is making noise, please take action".format(data[0]['temp']),
		    to="+919650055244",    # Replace with your phone number
		    from_="+12018905183") # Replace with your Twilio number
		print (message.sid)
	sleep(15)
