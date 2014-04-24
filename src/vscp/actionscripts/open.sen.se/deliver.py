import simplejson
import requests

class Sense:
    def __init__(self, senseKey):
        self.url        = 'http://api.sen.se'
        self.headers    = {'Content-Type':'application/json', 'sense_key':senseKey}

    def post(self,feedId, value):
        event = {
            'feed_id'   : feedId,
            'value'     : value
        }
        print "Now posting to Sen.se : %s" % event
        r = requests.post(
            self.url+'/events/',
            data=simplejson.dumps(event),
            headers=self.headers
        )
        if r.status_code != 200:
            print 'Could not post event. Api responded with a %s | %s' % (r.status, r.content)
        else:
            print 'Event posted to Sen.se'

if __name__ == "__main__":
    sense = Sense('YOUR_SEN.SE_KEY')
    sense.post(YOUR_FEED_ID, THE_VALUE)
    exit('Bye bye')
    
