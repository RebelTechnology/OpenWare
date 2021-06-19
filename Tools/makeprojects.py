#! /usr/bin/env python3

import itertools
import multiprocessing
import os
import shutil
import subprocess

PROJECTS = {
    'MidiBootOwl': [
        {
            'OwlPedal': ['Modular=OwlModular', 'Pedal=OwlPedal']
        }
    ],
    'MidiBoot': [
        'Alchemist', 'Wizard', 'Magus', 'Lich', 'Noctua',
        'BioSignals', 'Witch'
        #'Effectsbox' - won't build bootloader
    ],
    'MidiBoot3': [
        'Genius'
    ],
    #None: ['Expander'] - expander currently won't build
    #, 'MagusEncoder' - no F0 HAL in libraries
}


def get_projects():
    dirs = [
        dirname for dirname in os.listdir('.')
        if os.path.isdir(dirname)]

    for bootloader, devices_list in PROJECTS.items():
        for devices in devices_list:
            if isinstance(devices, dict):
                for key, values in devices.items():
                    for value in values:
                        yield bootloader, key, value
            else:
                yield bootloader, devices, devices

def get_platform_alias(platform):
    if '=' in platform:
        platform, platform_alias = platform.split('=')
    else:
        platform_alias = platform    
    return platform, platform_alias

def build_bootloader(dst, bootloader, data):
    print()
    for project, platform in data:
        platform, platform_alias = get_platform_alias(platform)
        print(
            f'{bootloader} for {project}'
            if platform == project else
            f'{bootloader} for {project}-{platform}')
        env = os.environ.copy()
        env['PLATFORM'] = platform
        subprocess.run(
            ['make', 'clean', 'all', 'sysex'], cwd=bootloader, env=env)
        shutil.copy(
            f'{bootloader}/Build/MidiBoot-{platform}.bin',
            f'{dst}/MidiBoot-{platform_alias}.bin')
        shutil.copy(
            f'{bootloader}/Build/MidiBoot-{platform}.syx',
            f'{dst}/MidiBoot-{platform_alias}.syx')

def build_project(dst, project, platforms):
    print()
    for platform in platforms:
        env = os.environ.copy()
        platform, platform_alias = get_platform_alias(platform)
        env['PLATFORM'] = platform
        print(
            f'Firmware for {platform}'
            if project == platform else
            f'Firmware for {project}-{platform}')
        subprocess.run(
            ['make', 'clean', 'all', 'sysex'],
            cwd=project, env=env)        
        shutil.copy(
            f'{project}/Build/{platform_alias}.bin',
            f'{dst}/{platform_alias}.bin')
        shutil.copy(
            f'{project}/Build/{platform_alias}.syx',
            f'{dst}/{platform_alias}.syx')

def get_version():
    for line in open('Source/device.h'):
        if line.startswith('#define FIRMWARE_VERSION'):
            words = [word for word in line.split() if word.strip()]
            return words[2].strip('"')
    else:
        raise Exception('Firmware version not found')

def main(config, smp):
    os.environ['CONFIG'] = config
    print(f'Config set to {config}')

    version = get_version()
    print(f'Firmware version {version}')

    dst = f'Build/{config}/{version}'
    os.makedirs(dst, exist_ok=True)

    data = list(get_projects())
    processes = []
    for bootloader, items in itertools.groupby(
            iter(data), key=lambda item: item[0]):
        if bootloader is not None:
            if smp:
                p = multiprocessing.Process(
                    target=build_bootloader,
                    args=(dst, bootloader, list(item[1:] for item in items)))
                p.start()
                processes.append(p)
            else:
                build_bootloader(dst, bootloader, list(item[1:] for item in items))

    for project, items in itertools.groupby(
            iter(data), key=lambda item: item[1]):
        if smp:
            p = multiprocessing.Process(
                target=build_project,
                args=(dst, project, list(item[2] for item in items)))
            p.start()
            processes.append(p)
        else:
            build_project(dst, project, list(item[2] for item in items))
            
    for p in processes:
        p.join()

    print()
    print(f'Results stored in Build/{config}/{version}/')

if __name__ == '__main__':
    import argparse
    argparser = argparse.ArgumentParser()
    argparser.add_argument('-p', '--parallel', action='store_true', help='Parallel build')
    args = argparser.parse_args()

    config = os.environ.get('CONFIG') or 'Release'

    main(config, args.parallel)
