import argparse, requests, json, os, base64

FILES = ['test_R2020a.ctf', 'test_R2021a.ctf', 'pace_neutrons_installer.exe', 'pace_neutrons_installer.zip']
GIST_ID = ['7389c37c89aed787c8a200c993fb67af', 'b595469518fbec7d9e293cf98bc123d6',
           'd13ac4e315dc0e0b95506c2b8bb4ef2d', '6a1606ced1d38118ec0e8d230048f372']

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--get', action='store_true', help='Retrieves stored ctf files')
    parser.add_argument('--set', action='store_true', help='Uploads ctf files to store')
    parser.add_argument('--token', action='store', help='Github token to access gist')
    parser.add_argument('--create', action='store_true', help='Creates and upload gist')
    args = parser.parse_args()

    token = args.token
    if token is None and 'GITHUB_TOKEN' in os.environ:
        token = os.environ['GITHUB_TOKEN']

    if not args.get and not args.set and not args.create:
        raise RuntimeError('One of the options --set, --get or --create must be specified')

    if args.get:
        get_gist()
    elif args.set:
        set_gist(token)
    else:
        create_gist(token)


def list_gist(token):
    assert token is not None, 'Need token for this action'
    response = requests.get(
        'https://api.github.com/gists',
        headers={'Accept': 'application/vnd.github+json', 'Authorization': 'Bearer ' + token})
    resp = json.loads(response.text)


def get_gist():
    headers={'Accept': 'application/vnd.github+json'}
    for gid in GIST_ID:
        response = requests.get(
            'https://api.github.com/gists/' + gid,
            headers=headers)
        assert response.status_code == 200, 'Could not download gist'
        jsout = json.loads(response.text) 
        for fname in jsout['files'].keys():
            print(f'Downloading file {fname}')
            if jsout['files'][fname]['truncated'] == True:
                response = requests.get(jsout['files'][fname]['raw_url'])
                assert response.status_code == 200, 'Could not download gist'
                content = response.text
            else:
                content = jsout['files'][fname]['content']
            path = os.path.join(os.path.dirname(__file__), 'test', fname)
            with open(path, 'wb') as f:
                f.write(base64.b85decode(content))


def set_gist(token):
    assert token is not None, 'Need token for this action'
    headers={'Accept': 'application/vnd.github+json', 'Authorization': 'Bearer ' + token}
    for idx, gid in enumerate(GIST_ID):
        path = os.path.join(os.path.dirname(__file__), 'test', FILES[idx])
        with open(path, 'rb') as f:
            contents = base64.b85encode(f.read()).decode('ascii')
        response = requests.patch(
            'https://api.github.com/gists/' + gid,
            headers=headers,
            data=json.dumps({'files':{FILES[idx]:{'content':contents}}}))
        if response.status_code != 200:
            raise RuntimeError('Could not update gist')


def create_gist(token):
    assert token is not None, 'Need token for this action'
    headers = {'Accept': 'application/vnd.github+json', 'Authorization': 'Bearer ' + token}
    data0 = {'description': 'b85 encoded ctf for testing libpymcr', 'public': True}
    for ctf in FILES:
        path = os.path.join(os.path.dirname(__file__), 'test', ctf)
        with open(path, 'rb') as f:
            contents = base64.b85encode(f.read()).decode('ascii')
        response = requests.post(
            'https://api.github.com/gists',
            data=json.dumps({'files':{ctf:{'content':contents}}, **data0}),
            headers=headers)
        if response.status_code != 201:
            raise RuntimeError('Could not create gist')


if __name__ == '__main__':
    main()
