import argparse, requests, json, os, base64

FILES = ['test_R2021a.ctf', 'test_R2023a.ctf', 'test_R2024a.ctf', 'test_R2024b.ctf']
GIST_ID = ['b595469518fbec7d9e293cf98bc123d6', 'fda80b61074de6fb03530eb7ddaa82de',
           '2727bd8d670e42cd27e12d8c51582868', 'c7ca72b1c80a501edef1b6434a4983aa']

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--get', action='store_true', help='Retrieves stored ctf files')
    parser.add_argument('--set', action='store_true', help='Uploads ctf files to store')
    parser.add_argument('--token', action='store', help='Github token to access gist')
    parser.add_argument('--create', action='store_true', help='Creates and upload gist')
    parser.add_argument('--dir', action='store', help='Directory to store or get files')
    args = parser.parse_args()

    token = args.token
    if token is None and 'GITHUB_TOKEN' in os.environ:
        token = os.environ['GITHUB_TOKEN']

    if not args.get and not args.set and not args.create:
        raise RuntimeError('One of the options --set, --get or --create must be specified')

    inoutdir = 'test' if args.dir is None else args.dir

    if args.get:
        get_gist(token, inoutdir)
    elif args.set:
        set_gist(token, inoutdir)
    else:
        create_gist(token, inoutdir)

def list_gist(token):
    assert token is not None, 'Need token for this action'
    response = requests.get(
        'https://api.github.com/gists',
        headers={'Accept': 'application/vnd.github+json', 'Authorization': 'Bearer ' + token})
    resp = json.loads(response.text)


def get_gist(token=None, outputdir='test'):
    headers={'Accept': 'application/vnd.github+json'}
    if token is not None:
        headers = {'Authorization': 'Bearer ' + token, **headers}
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
            path = os.path.join(os.path.dirname(__file__), outputdir, fname)
            with open(path, 'wb') as f:
                f.write(base64.b85decode(content))


def set_gist(token, indir='test'):
    assert token is not None, 'Need token for this action'
    headers={'Accept': 'application/vnd.github+json', 'Authorization': 'Bearer ' + token}
    for idx, gid in enumerate(GIST_ID):
        path = os.path.join(os.path.dirname(__file__), indir, FILES[idx])
        with open(path, 'rb') as f:
            contents = base64.b85encode(f.read()).decode('ascii')
        response = requests.patch(
            'https://api.github.com/gists/' + gid,
            headers=headers,
            data=json.dumps({'files':{FILES[idx]:{'content':contents}}}))
        if response.status_code != 200:
            raise RuntimeError('Could not update gist')


def create_gist(token, indir='test'):
    assert token is not None, 'Need token for this action'
    headers = {'Accept': 'application/vnd.github+json', 'Authorization': 'Bearer ' + token}
    data0 = {'description': 'b85 encoded ctf for testing libpymcr', 'public': True}
    for ctf in FILES:
        path = os.path.join(os.path.dirname(__file__), indir, ctf)
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
