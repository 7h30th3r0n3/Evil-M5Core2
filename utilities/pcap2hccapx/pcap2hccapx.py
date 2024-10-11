#By 7h30th3r0n3

import os
import argparse
from subprocess import Popen, PIPE

#====================================
# Local Configuration
#====================================
config = {
    'local_dir': './',  # Set your local pcap directory
}

def grab_bssid(file):
    try:
        process = Popen(['tshark', '-r', os.path.join(config['local_dir'], file), '-T', 'fields', '-e', 'wlan.bssid'], stdout=PIPE, stderr=PIPE)
        stdout, _ = process.communicate()
        output = stdout.decode().splitlines()

        if output:
            return list(set(output)) 

        return None
    except Exception as e:
        print(f'Error occurred while grabbing BSSID: {e}')
        return None

def convert_file(file):
    try:
        print(f'Processing: {file}')

        process = Popen(['hcxpcapngtool', '-o', f'./pmkid/{file.replace(".pcap", "")}.pmkid', os.path.join(config['local_dir'], file)], stdout=PIPE, stderr=PIPE)
        _, stderr = process.communicate()

        if 'PMKID(s) written' in stderr.decode():
            print('Found PMKID')
            return True
        else:
            process = Popen(['hcxpcapngtool', '-o', f'./hccapx/{file.replace(".pcap", "")}.hccapx', os.path.join(config['local_dir'], file)], stdout=PIPE, stderr=PIPE)
            _, stderr = process.communicate()

            if 'handshake(s) written' in stderr.decode():
                print('Found Handshake')
                return True

        return ''
    except Exception as e:
        print(f'Error occurred while converting file: {e}')
        return None

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-r', '--remove', action='store_true', help='delete pcap files after processing')
    args = parser.parse_args()

    files = [f for f in os.listdir(config['local_dir']) if f.endswith('.pcap')]

    # If 'pmkid' directory doesn't exist, create it.
    if not os.path.exists('./pmkid'):
        os.mkdir('./pmkid')

    # If 'hccapx' directory doesn't exist, create it.
    if not os.path.exists('./hccapx'):
        os.mkdir('./hccapx')

    for file in files:
        bssids = grab_bssid(file)
        if not bssids:
            print(f'No BSSID found for {file}')
        else:
            for bssid in bssids:
                result = convert_file(file)
                if result is True:
                    print(f'Added {file} to database with BSSID: {bssid}')
                else:
                    print(result)

    if args.remove:
        for file in files:
            os.remove(os.path.join(config['local_dir'], file))
            print(f'Removed {file}')

    print('\n\n')
    print('===================')
    print('   All done.  ')
    print('===================')

if __name__ == '__main__':
    main()
